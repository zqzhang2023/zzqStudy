#include "../include/manageAll.h"

void audioFrameEncoding(){
    

    //重采样过程
    audioEncoderCtxMtx.lock();
    SwrContext *swrCtx = swr_alloc_set_opts(nullptr,
        audioEncoderCtx->channel_layout,//新声道布局
        audioEncoderCtx->sample_fmt,    //新采样格式
        audioEncoderCtx->sample_rate,   //新采样率
        audioCodecCtx->channel_layout,  //原始声道布局
        audioCodecCtx->sample_fmt,      //原始采样格式
        audioCodecCtx->sample_rate,     //原始采样率
        0, nullptr);
    audioEncoderCtxMtx.unlock();
    
    if (!swrCtx||swr_init(swrCtx)<0) {
        cout<<"无法初始化重采样器！"<< endl;
        return;
    }

    //处理帧的pts
    static int64_t audio_frame_pts = 0;
    AVFrame* resampledFrame = av_frame_alloc();

    //还在处理音频帧，或者audioFrameQueue非空
    while (!doneAudioFrameReading||!audioFrameBuffer.empty()){
        // 从队列中区取帧
        unique_lock<mutex> frameLock(audioFrameMtx);        //上锁
        audioFrameCV.wait(frameLock, [&] {return !audioFrameBuffer.empty() || doneAudioFrameReading;});  //空等待
        if(!audioFrameBuffer.empty()) {
            AVFrame* frame = audioFrameBuffer.front();
            audioFrameBuffer.pop();
            frameLock.unlock();
            audioFrameCV.notify_all();                     //这里一定要注意通知环形缓冲区，有空了！！！
            frame->pts = audio_frame_pts;                  //每帧递增！！！一定要注意
            //重采样过程（编码格式，帧率等）
            audioEncoderCtxMtx.lock();
            resampledFrame->format = audioEncoderCtx->sample_fmt;
            resampledFrame->channel_layout = audioEncoderCtx->channel_layout;
            resampledFrame->sample_rate = audioEncoderCtx->sample_rate;
            resampledFrame->nb_samples = audioEncoderCtx->frame_size;
            resampledFrame->channels = audioEncoderCtx->channels;
            resampledFrame->pts = audio_frame_pts;         // 设置PTS!!!一定要注意
            audioEncoderCtxMtx.unlock();

            //分配缓冲区空间
            av_frame_get_buffer(resampledFrame, 0);

            //转换样本
            int converted = swr_convert(swrCtx,
                resampledFrame->data, resampledFrame->nb_samples,
                (const uint8_t**)frame->data, frame->nb_samples);
            
            //converted为转换成功的样本数
            if (converted<0) {
                cout<<"重采样失败！"<<endl;
                av_frame_free(&frame);
                continue;
            }
            audio_frame_pts += converted;  //更新PTS（+转换后的样本数）


            audioEncoderCtxMtx.lock();
            if (avcodec_send_frame(audioEncoderCtx,resampledFrame)<0) {
                av_frame_free(&frame);
                cout<<"音频，发送帧到编码器失败！"<<endl;
                audioEncoderCtxMtx.unlock();
                continue;
            }
            audioEncoderCtxMtx.unlock();
            av_frame_free(&frame); // 释放原始帧
            av_frame_unref(resampledFrame);

            AVPacket* encodedPacket = av_packet_alloc();   //临时变量
            audioEncoderCtxMtx.lock();
            int receivePacketResult = avcodec_receive_packet(audioEncoderCtx, encodedPacket);
            audioEncoderCtxMtx.unlock();
            while (receivePacketResult == 0) {
                AVPacket* pkt = av_packet_clone(encodedPacket);            //克隆包并存入队列
                unique_lock<mutex> encLock(audioFrameEncoderMtx);          //上锁
                audioFrameEncoderQueue.push(pkt);                          //push   结束了开锁
                encLock.unlock();
                audioFrameEncoderCV.notify_all();                          // 通知有新包
                av_packet_unref(encodedPacket);
                audioEncoderCtxMtx.lock();
                receivePacketResult = avcodec_receive_packet(audioEncoderCtx, encodedPacket);
                audioEncoderCtxMtx.unlock();
            }
            //释放资源
            av_packet_free(&encodedPacket); 
        }

    }
    

    //清空最后的缓冲区
    audioEncoderCtxMtx.lock();
    avcodec_send_frame(audioEncoderCtx, nullptr);
    audioEncoderCtxMtx.unlock();

    AVPacket* encodedPacket = av_packet_alloc();
    while (true) {
        audioEncoderCtxMtx.lock();
        int receivePacketResult = avcodec_receive_packet(audioEncoderCtx, encodedPacket);
        audioEncoderCtxMtx.unlock();
    
        if (receivePacketResult == AVERROR(EAGAIN)||receivePacketResult == AVERROR_EOF) {
            break;  //如果没有更多包可接收，退出循环
        } else if (receivePacketResult < 0) {   //处理错误
            av_packet_free(&encodedPacket);
            break;
        }
        //处理编码后的包
        AVPacket* pkt = av_packet_clone(encodedPacket);
        unique_lock<mutex> encLock(audioFrameEncoderMtx);
        audioFrameEncoderQueue.push(pkt);
        encLock.unlock();
        audioFrameEncoderCV.notify_all();
        av_packet_unref(encodedPacket);
    }

    av_packet_free(&encodedPacket);
    
    //都结束了
    doneAudioFrameEncoderReading = true;
    audioFrameEncoderCV.notify_all();

    av_frame_free(&resampledFrame);
    swr_free(&swrCtx);

    cout<<"--线程编码音频帧完成！--"<<endl;
}