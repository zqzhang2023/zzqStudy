#include "../include/manageAll.h"
void processAudioPackets(){
    
    //临时的frame 和 packet用来接收
    AVFrame* frame = av_frame_alloc();
    AVPacket* packet = av_packet_alloc();

    //如果还在push视频或者audioPacketQueue还没有空，就要一直执行
    while (!doneAudioReading || !audioPacketQueue.empty()) {
        unique_lock<std::mutex> lock(audioMtx);    //上锁（队列）
        audioCV.wait(lock, [] { return !audioPacketQueue.empty() || doneAudioReading; });  //等待
        if (!audioPacketQueue.empty()) {
            *packet = *audioPacketQueue.front();   //取包并pop
            audioPacketQueue.pop();
            lock.unlock();
            //unique_lock<mutex> audioCodecCtxLock(audioCodecCtxMtx);
            if (avcodec_send_packet(audioCodecCtx, packet) == 0) {
                while (avcodec_receive_frame(audioCodecCtx, frame) == 0) {
                    AVFrame* frameCopy = av_frame_alloc();     //复制帧数据到新帧 防止当前帧正在使用出现资源争夺
                    if (av_frame_ref(frameCopy, frame) != 0) {
                        av_frame_free(&frameCopy);
                        cout<<"音频 复制失败"<<endl;
                        continue;                               //复制失败则跳过该帧
                    }
                    
                    unique_lock<mutex> frameLock(audioFrameMtx);//将复制的帧推入环形缓冲区
                    //if (audioFrameBuffer.full()){   //做调试输出
                    //    cout<<"音频环形缓冲区满了，一直等待"<<endl;
                    //}
                    // 等待缓冲区有空位
                    audioFrameCV.wait(frameLock, [&] {
                        return !audioFrameBuffer.full();
                    });
                    audioFrameBuffer.push(frameCopy);
                    audioFrameCV.notify_all(); // 有新帧 唤醒
                }
                av_packet_unref(packet);
            }
        }
    }

    doneAudioFrameReading = true;
    audioFrameCV.notify_all();
    //释放资源
    av_frame_free(&frame);
    av_packet_free(&packet);
    
    cout<<"--线程解析音频包完成！--"<<endl;
}