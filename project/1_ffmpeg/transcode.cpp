#include<iostream>
#include "lib/include/manageAll.h"

int main(){

    //源视频文件
    const char* filePath = "../resources/1.mp4";
    //最后输出的文件视频
    const char* outFilePatMP4VideoPath = "../resources/2.mp4";

    cout<<"该程序需要拆分一个MP4音视频为视频和音频的包，并对包进行解码，针对解码后的帧加上特效，最后再合成一个新的MP4音视频"<<endl;

    cout<<"初始化全局变量......"<<endl;
    int initResult = initGlobalVariable(filePath);
    if (initResult==0){
        cout<<"初始化成功!"<<endl;
    }else{
        cout<<"初始化失败!"<<endl;
    }
    
    cout<<"------------------------------------------"<<endl;
    
    thread readerThread(productionPackage);                  //视频获取包的线程
    thread processorVideoThread(processVideoPackets);        //视频解码
    thread videoFrameEncodingThread(videoFrameEncoding);     //视频编码
    thread processorAudioThread(processAudioPackets);        //音频解码
    thread audioFrameEncodingThread(audioFrameEncoding);     //音频编码
    thread videoOutMP4FileThread(videoOutMP4File,outFilePatMP4VideoPath);    //输出文件

    readerThread.join();
    processorVideoThread.join();
    videoFrameEncodingThread.join();
    processorAudioThread.join();
    audioFrameEncodingThread.join();
    videoOutMP4FileThread.join();
    cout<<"------------------------------------------"<<endl;

    //释放掉所有的东西 统一释放，因此在各个文件之中的一些异常管理之中就不需要手动释放了，因为最后会统一释放
    releaseResources();

    cout<<"全局资源已经释放了..."<<endl;


    return 0;
}