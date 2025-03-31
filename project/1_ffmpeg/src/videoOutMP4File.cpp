#include "../include/manageAll.h"

void videoOutMP4File(const char* outFilePath){

    videoEncoderCtxMtx.lock();
    audioEncoderCtxMtx.lock();
    //初始化输出 MP4 文件
    AVFormatContext* outFormatCtx = nullptr;
    avformat_alloc_output_context2(&outFormatCtx, nullptr, nullptr, outFilePath);
    if (!outFormatCtx) {
        videoEncoderCtxMtx.unlock();
        audioEncoderCtxMtx.unlock();
        cout << "无法创建输出上下文" << endl;
        return;
    }

    //创建视频流
    AVStream* videoStream = avformat_new_stream(outFormatCtx, nullptr);
    avcodec_parameters_from_context(videoStream->codecpar, videoEncoderCtx);
    videoStream->time_base = videoEncoderCtx->time_base;

    //创建音频流
    AVStream* audioStream = avformat_new_stream(outFormatCtx, nullptr);
    avcodec_parameters_from_context(audioStream->codecpar, audioEncoderCtx);
    audioStream->time_base = audioEncoderCtx->time_base;

    //打开输出文件
    if (avio_open(&outFormatCtx->pb, outFilePath, AVIO_FLAG_WRITE) < 0) {
        cout<<"无法打开输出文件"<<endl;
        videoEncoderCtxMtx.lock();
        audioEncoderCtxMtx.lock();
        avformat_free_context(outFormatCtx);
        return;
    }
    
    //写入文件头
    if (avformat_write_header(outFormatCtx, nullptr) < 0) {
        cout << "无法写入文件头" << endl;
        videoEncoderCtxMtx.lock();
        audioEncoderCtxMtx.lock();
        avio_closep(&outFormatCtx->pb);
        avformat_free_context(outFormatCtx);
        return;
    }

    //用完了解锁 上锁和解锁的顺序是反者的，防止死锁
    audioEncoderCtxMtx.unlock();
    videoEncoderCtxMtx.unlock();

    //临时变量
    AVPacket* videoPacket = av_packet_alloc();
    AVPacket* audioPacket = av_packet_alloc();

    //循环退出条件  只有当两个队列都是空的并且对视频和音频的编码的操作也都结束了才会停止 确保所有的数据都写进去
    bool videoDone = videoFrameEncoderQueue.empty() && doneVideoFrameEncoderReading;
    bool audioDone = audioFrameEncoderQueue.empty() && doneAudioFrameEncoderReading;
    while (!videoDone||!audioDone) {
        
        AVPacket* packetToWrite = nullptr;
        int streamIndex = -1;
        {
            unique_lock<mutex> videoQueue(videoFrameEncoderMtx);
            unique_lock<mutex> audioQueue(audioFrameEncoderMtx);


            //等待两个队列都有数据 或者两个队列都已经push完了，要保证两个队列之中都有数据
            videoFrameEncoderCV.wait(videoQueue,[]{return !videoFrameEncoderQueue.empty()||doneVideoFrameEncoderReading; });
            audioFrameEncoderCV.wait(audioQueue,[]{return !audioFrameEncoderQueue.empty()||doneAudioFrameEncoderReading; });

            //获取音视频当前时间戳，要放在确定的时间基下
            int64_t video_pts = (!videoFrameEncoderQueue.empty()) ? 
                av_rescale_q(videoFrameEncoderQueue.front()->pts,videoEncoderCtx->time_base,videoStream->time_base):INT64_MAX;
            int64_t audio_pts = (!audioFrameEncoderQueue.empty()) ? 
                av_rescale_q(audioFrameEncoderQueue.front()->pts,audioEncoderCtx->time_base,audioStream->time_base):INT64_MAX;


            //选择时间戳较小的帧进行写入
            if (!videoFrameEncoderQueue.empty()&&
            (audioFrameEncoderQueue.empty()||
            av_compare_ts(video_pts,videoStream->time_base,audio_pts,audioStream->time_base)<=0)) {     //通过av_compare_ts作比较（不同的时间基下）
                *videoPacket = *videoFrameEncoderQueue.front();                                         //取包
                videoFrameEncoderQueue.pop();                                                           //pop
                av_packet_rescale_ts(videoPacket, videoEncoderCtx->time_base, videoStream->time_base);  //转化到对应的时间基下
                videoPacket->stream_index = videoStream->index;                                         //设置stream_index（视频的）
                packetToWrite = videoPacket;                                                            //拷贝过来（这里还没有写入）
                streamIndex = videoStream->index;                                                       //当前的index（后面是醋糊错误信息用）
            } else if (!audioFrameEncoderQueue.empty()) {                                               //下面是一样的流程
                *audioPacket = *audioFrameEncoderQueue.front();
                audioFrameEncoderQueue.pop();
                av_packet_rescale_ts(audioPacket, audioEncoderCtx->time_base, audioStream->time_base);
                audioPacket->stream_index = audioStream->index;
                packetToWrite = audioPacket;
                streamIndex = audioStream->index;
            }
            //while结束条件 只有当两个队列都是空的并且对视频和音频的编码的操作也都结束了才会停止
            videoDone = videoFrameEncoderQueue.empty() && doneVideoFrameEncoderReading;
            audioDone = audioFrameEncoderQueue.empty() && doneAudioFrameEncoderReading;
            audioQueue.unlock();
            videoQueue.unlock();
        }

        if (packetToWrite) {  //这里一定要注意不可以视频和音频是交叉写入的，这样才是正确的效果（之前老师让我哦门使用CSV输出的信息的时候可以观察到）
            if (av_interleaved_write_frame(outFormatCtx, packetToWrite) < 0) {
                cout<<"写入包失败: "<<streamIndex<<endl;
            }
            av_packet_unref(packetToWrite);
        }

    }

    //释放资源
    videoEncoderCtxMtx.lock();
    avcodec_send_frame(videoEncoderCtx, nullptr);
    videoEncoderCtxMtx.unlock();

    audioEncoderCtxMtx.lock();
    avcodec_send_frame(audioEncoderCtx, nullptr);
    audioEncoderCtxMtx.unlock();

    av_packet_free(&videoPacket);
    av_packet_free(&audioPacket);

    av_write_trailer(outFormatCtx);
    avio_closep(&outFormatCtx->pb);
    avformat_free_context(outFormatCtx);

    cout<<"--线程写入MP4完成！--"<<endl;
}