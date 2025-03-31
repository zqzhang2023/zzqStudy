#include "../include/manageAll.h"

void videoFrameEncoding(){


    static int64_t frame_pts = 0;
    //还在处理视频帧，或者videoFrameBuffer非空
    while (!doneVideoFrameReading||!videoFrameBuffer.empty()){
        // 从队列中区取帧
        unique_lock<mutex> frameLock(videoFrameMtx);    //上锁
        videoFrameCV.wait(frameLock, [&] {return !videoFrameBuffer.empty() || doneVideoFrameReading;});  //空等待
        if(!videoFrameBuffer.empty()) {
            AVFrame* frame = videoFrameBuffer.front();
            videoFrameBuffer.pop();
            frame->pts = frame_pts++;        //！！！很关键的pts设置
            frameLock.unlock();
            videoFrameCV.notify_all();       //环形缓冲区有空间了
            videoEncoderCtxMtx.lock();       //videoEncoderCtxMtx的锁，防止和输出的部分有竞争
            if (avcodec_send_frame(videoEncoderCtx,frame)<0){
                av_frame_free(&frame);
                cout<<"视频，发送帧到编码器失败！"<<endl;
                continue;
            }
            videoEncoderCtxMtx.unlock();

            av_frame_free(&frame); //释放帧资源
            
            AVPacket* encodedPacket = av_packet_alloc();
            videoEncoderCtxMtx.lock();
            int receivePacketResult = avcodec_receive_packet(videoEncoderCtx, encodedPacket);
            videoEncoderCtxMtx.unlock();
            while (receivePacketResult == 0) {
                AVPacket* pkt = av_packet_clone(encodedPacket);     //克隆包并存入队列
                unique_lock<mutex> encLock(videoFrameEncoderMtx);   //上锁
                videoFrameEncoderQueue.push(pkt);                   //push   结束了开锁
                encLock.unlock();
                videoFrameEncoderCV.notify_all();                    // 通知有新包
                av_packet_unref(encodedPacket);
                videoEncoderCtxMtx.lock();
                receivePacketResult = avcodec_receive_packet(videoEncoderCtx, encodedPacket);
                videoEncoderCtxMtx.unlock();
            }
            //释放资源
            av_packet_free(&encodedPacket);
        }

    }

    //清空最后的缓冲区
    videoEncoderCtxMtx.lock();
    avcodec_send_frame(videoEncoderCtx, nullptr);
    videoEncoderCtxMtx.unlock();

    AVPacket* encodedPacket = av_packet_alloc();
    while (true) {
        videoEncoderCtxMtx.lock();
        int receivePacketResult = avcodec_receive_packet(videoEncoderCtx, encodedPacket);
        videoEncoderCtxMtx.unlock();
    
        if (receivePacketResult == AVERROR(EAGAIN)||receivePacketResult == AVERROR_EOF) {
            break;  //如果没有更多包可接收，退出循环
        } else if (receivePacketResult < 0) {   //处理错误
            av_packet_free(&encodedPacket);
            break;
        }
        //处理编码后的包
        AVPacket* pkt = av_packet_clone(encodedPacket);
        unique_lock<mutex> encLock(videoFrameEncoderMtx);
        videoFrameEncoderQueue.push(pkt);
        encLock.unlock();
        videoFrameEncoderCV.notify_all();
        av_packet_unref(encodedPacket);
    }

    //都结束了
    doneVideoFrameEncoderReading = true;
    videoFrameEncoderCV.notify_all();

    cout<<"--线程编码视频帧完成！--"<<endl;
}