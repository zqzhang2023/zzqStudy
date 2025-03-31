#include "../include/manageAll.h"

void processVideoPackets() {
    
    //初始化临时的负责接收的包和帧
    AVFrame* frame = av_frame_alloc();
    AVPacket* packet = av_packet_alloc();

    //如果还在push视频或者videoPacketQueue还没有空，就要一直执行
    while (!doneVideoReading || !videoPacketQueue.empty()) {
        std::unique_lock<std::mutex> lock(videoMtx);   //上锁（队列）
        videoCV.wait(lock, [] { return !videoPacketQueue.empty() || doneVideoReading; });  //等待（队列）
        if (!videoPacketQueue.empty()) {              //videoPacketQueue还没空
            *packet = *videoPacketQueue.front();      //并且取头pop出去
            videoPacketQueue.pop();
            lock.unlock();                            //用完了解锁
            if (avcodec_send_packet(videoCodecCtx, packet) == 0) {      
                while (avcodec_receive_frame(videoCodecCtx, frame) == 0){
                    AVFrame* frameCopy = av_frame_alloc();  // 复制帧数据到新帧 防止当前帧正在使用出现资源争夺
                    if (av_frame_ref(frameCopy, frame) != 0) {
                        av_frame_free(&frameCopy);
                        cout<<"视频 复制失败"<<endl;
                        continue; // 复制失败则跳过该帧
                    }
                    // 将复制的帧推入环形缓冲区
                    unique_lock<mutex> frameLock(videoFrameMtx);
                    //这里是用来输出查看环形缓冲区是不是正常的
                    //if (videoFrameBuffer.full()){
                        //cout<<"视频环形缓冲区满了，一直等待"<<endl;
                    //}
                    //等待缓冲区有空位  
                    videoFrameCV.wait(frameLock, [&] {
                        return !videoFrameBuffer.full();
                    });
                    videoFrameBuffer.push(frameCopy);
                    videoFrameCV.notify_all(); // 有新帧 唤醒
                }
            }
            av_packet_unref(packet);
        }
    }

    doneVideoFrameReading = true;
    videoFrameCV.notify_all();
    //关文件，放资源
    av_frame_free(&frame);
    av_packet_free(&packet);
    
    cout<<"--线程解析视频包完成！--"<<endl;
}