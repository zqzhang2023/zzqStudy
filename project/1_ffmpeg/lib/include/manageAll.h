#ifndef DECODER_H
#define DECODER_H

#include <iostream>
#include <fstream>
#include <cstdint>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
extern "C" {
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
    #include "libavutil/avutil.h"
    #include "libswresample/swresample.h"
    #include "libavutil/opt.h"
    #include "libavutil/channel_layout.h"
    #include "libavutil/channel_layout.h"
    #include "libavfilter/avfilter.h"
    #include "libavfilter/buffersink.h"
    #include "libavfilter/buffersrc.h"
}
#include "queue.h"
#include "circularbuffer.h"



extern AVFormatContext* formatCtx;
//视频全局变量----------------------------------------------------
extern Queue<AVPacket*> videoPacketQueue;            //视频包队列
extern mutex videoMtx;                               //队列的锁
extern condition_variable videoCV;                   //条件变量
extern atomic<bool> doneVideoReading;                //判断是否在读视频（方便处理wait函数）

extern int vsIndex;                                  //视频流index
extern AVCodecParameters* videoCodecParams;          //视频流编码器参数
extern const AVCodec* videoCodec;                    //视频流编解码器
extern AVCodecContext* videoCodecCtx;                //解码器上下文

extern CircularBuffer<AVFrame*,100> videoFrameBuffer; //视频环形缓冲区队列
extern mutex videoFrameMtx;                           //保护videoFrameBuffer的互斥锁
extern condition_variable videoFrameCV;               //条件变量
extern atomic<bool> doneVideoFrameReading;            //标志处理是否完成

extern Queue<AVPacket*> videoFrameEncoderQueue;       //编码后的包的队列
extern mutex videoFrameEncoderMtx;                    //队列的锁
extern condition_variable videoFrameEncoderCV;        //条件变量
extern atomic<bool> doneVideoFrameEncoderReading;     //判断是否在编码（方便处理wait函数）

extern const AVCodec* videoEncoder;                   //视频编码器
extern AVCodecContext* videoEncoderCtx;               //编码器上下文
extern condition_variable videoEncoderCtxCV;          //videoEncoderCtx的条件变量
extern mutex videoEncoderCtxMtx;                      //videoEncoderCtx的锁

//音频全局变量----------------------------------------------------
extern Queue<AVPacket*> audioPacketQueue;             //音频流队列
extern mutex audioMtx;                                //队列的锁
extern condition_variable audioCV;                    //条件变量
extern atomic<bool> doneAudioReading;                 //判断是否在读音频（方便处理wait函数）

extern int asIndex;                                   //音频index 
extern AVCodecParameters* audioCodecParams;           //音频流编码器参数
extern const AVCodec* audioCodec;                     //音频流编解码器
extern AVCodecContext* audioCodecCtx;                 //编解码器上下文
extern condition_variable audioCodecCtxCV;            //audioCodecCtx的条件变量
extern mutex audioCodecCtxMtx;                        //audioCodecCtx的锁哦

extern CircularBuffer<AVFrame*,100> audioFrameBuffer; //音频环形缓冲区队列
extern mutex audioFrameMtx;                           //保护audioFrameBuffer的互斥锁
extern condition_variable audioFrameCV;               //用于缓冲区的条件变量
extern atomic<bool> doneAudioFrameReading;            //标志处理是否完成

extern Queue<AVPacket*> audioFrameEncoderQueue;       //编码后的音频的包的队列
extern mutex audioFrameEncoderMtx;                    //队列的锁
extern condition_variable audioFrameEncoderCV;        //条件变量
extern atomic<bool> doneAudioFrameEncoderReading;     //判断是否在编码（方便处理wait函数）

extern AVCodec *audioEncoder;                        //音频编码器
extern AVCodecContext *audioEncoderCtx;              //编码器上下文
extern condition_variable audioEncoderCtxCV;         //audioEncoderCtxCV的条件变量
extern mutex audioEncoderCtxMtx;                     //audioEncoderCtxCV的锁


//倍速播放的倍数
extern double playSpeed;    



//函数声明----------------------------------------------------
int  initGlobalVariable(const char* filePath);        //初始化参数（正常返回0）
void releaseResources();                              //释放参数
void productionPackage();                             //将包推进队列
void processVideoPackets();                           //解析视频包
void processAudioPackets();    //解析音频包
void videoFrameEncoding();                            //对视频帧进行编码
void audioFrameEncoding();                            //对音频帧进行编码
void videoOutMP4File(const char* outFilePath);        //输出最后的文件

#endif // DECODER