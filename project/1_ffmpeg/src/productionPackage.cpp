#include "../include/manageAll.h"

void productionPackage(){


    // 分配包
    AVPacket* packet = av_packet_alloc();
    //一直读包
    while (av_read_frame(formatCtx, packet) >= 0) {
        if (packet->stream_index == vsIndex) {        // 处理视频包
            lock_guard<mutex> lock(videoMtx);         //上锁
            AVPacket* videoPkt = av_packet_alloc();   //这里要注意，packet一直在被使用，不能直接push，否则会和解析的线程抢占这个packet
            av_packet_ref(videoPkt, packet);          //复制一份再push进去
            videoPacketQueue.push(videoPkt);          //push
            videoCV.notify_one();                     //唤醒
        } else if (packet->stream_index == asIndex) { // 处理音频包
            lock_guard<mutex> lock(audioMtx);         //上锁
            AVPacket* audioPkt = av_packet_alloc();   //这里要注意，packet一直在被使用，不能直接push，否则会和解析的线程抢占这个packet
            av_packet_ref(audioPkt, packet);          //复制一份再push进去
            audioPacketQueue.push(audioPkt);          //push
            audioCV.notify_one();                     //唤醒
        }
        av_packet_unref(packet); //释放当前包的引用
    }

    
    av_packet_free(&packet);     //循环结束后释放packet
    doneVideoReading = true;     //视频读完了
    videoCV.notify_all();        //唤醒
    doneAudioReading = true;     //音频读完了
    audioCV.notify_all();        //唤醒

    cout<<"--线程获取包执行完成！--"<<endl;
}