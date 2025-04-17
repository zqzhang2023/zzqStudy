#include "../include/manageAll.h"


AVFormatContext* formatCtx = nullptr;
//视频全局变量----------------------------------------------------
Queue<AVPacket*> videoPacketQueue;                 //视频包队列
mutex videoMtx;                                    //队列的锁
condition_variable videoCV;                        //条件变量
atomic<bool> doneVideoReading(false);              //判断是否在读视频（方便处理wait函数）

int vsIndex = -1;                                  //视频流index
AVCodecParameters* videoCodecParams = nullptr;     //视频流编码器参数
const AVCodec* videoCodec = nullptr;               //视频流编解码器
AVCodecContext* videoCodecCtx = nullptr;           //解码器上下文

CircularBuffer<AVFrame*,100> videoFrameBuffer;     //视频环形缓冲区队列
mutex videoFrameMtx;                               //保护videoFrameBuffer的互斥锁
condition_variable videoFrameCV;                   //条件变量
atomic<bool>  doneVideoFrameReading(false);        //标志处理是否完成

Queue<AVPacket*> videoFrameEncoderQueue;           //视频帧队列
mutex videoFrameEncoderMtx;                        //队列的锁
condition_variable videoFrameEncoderCV;            //条件变量
atomic<bool>  doneVideoFrameEncoderReading(false); //判断是否在编码（方便处理wait函数）

const AVCodec* videoEncoder = nullptr;             //视频编码器
AVCodecContext* videoEncoderCtx = nullptr;         //编码器上下文
condition_variable videoEncoderCtxCV;              //videoEncoderCtx的条件变量
mutex videoEncoderCtxMtx;                          //videoEncoderCtx的锁哦

//音频全局变量----------------------------------------------------
Queue<AVPacket*> audioPacketQueue;                 //音频流队列
mutex audioMtx;                                    //队列的锁
condition_variable audioCV;                        //条件变量
atomic<bool>  doneAudioReading(false);             //判断是否在读音频（方便处理wait函数）

int asIndex = -1;                                  //音频index  
AVCodecParameters* audioCodecParams = nullptr;     //音频解码码器参数
const AVCodec* audioCodec = nullptr;               //音频流解码器
AVCodecContext* audioCodecCtx = nullptr;           //编解码器上下文
condition_variable audioCodecCtxCV;                //audioCodecCtx的条件变量
mutex audioCodecCtxMtx;                            //audioCodecCtx的锁哦

CircularBuffer<AVFrame*,100> audioFrameBuffer;     //音频环形缓冲区队列
mutex audioFrameMtx;                               //保护audioFrameBuffer的互斥锁
condition_variable audioFrameCV;                   //用于缓冲区的条件变量
atomic<bool>  doneAudioFrameReading(false);        //标志处理是否完成

Queue<AVPacket*> audioFrameEncoderQueue;           //编码后的音频的包的队列
mutex audioFrameEncoderMtx;                        //队列的锁
condition_variable audioFrameEncoderCV;            //条件变量
atomic<bool>  doneAudioFrameEncoderReading(false); //判断是否在编码（方便处理wait函数）

AVCodec *audioEncoder = nullptr;                   //音频编码器
AVCodecContext *audioEncoderCtx = nullptr;         //上下文参数
condition_variable audioEncoderCtxCV;              //audioEncoderCtxCV的条件变量
mutex audioEncoderCtxMtx;                          //audioEncoderCtxCV的锁


//倍速播放
double playSpeed = 1.0;

int initGlobalVariable(const char* filePath){
    // 打开输入文件
    int openResult = avformat_open_input(&formatCtx,filePath,nullptr,nullptr);
    if (openResult!=0) {
        cout<<"不可以打开视频文件"<<filePath<<endl;
        return -1;
    }
    // 获取流信息
    int findStreaResuly = avformat_find_stream_info(formatCtx,nullptr);
    if(findStreaResuly<0) {
        cout<<"无法找到流信息"<<endl;
        return -2;
    }
    // 查找视频流   AVMEDIA_TYPE_VIDEO 为视频流 AVMEDIA_TYPE_AUDIO 音频流
    for (int i=0;i<formatCtx->nb_streams;i++) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            vsIndex = i;
            break;
        }
    }
    if (vsIndex==-1) {
        cout<<"无法找到视频流"<<endl;
        return -3;
    }

    // 查找音频流   AVMEDIA_TYPE_VIDEO 为视频流 AVMEDIA_TYPE_AUDIO 音频流
    for (int i=0;i<formatCtx->nb_streams;i++) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            asIndex = i;
            break;
        }
    }
    if (asIndex==-1) {
        cout<<"无法找到音频流"<<endl;
        return -4;
    }
     
    // 获取视频流的编解码器参数
    videoCodecParams = formatCtx->streams[vsIndex]->codecpar;
    if (!videoCodecParams) {
        cout<<"无法获取编码器参数-视频"<<endl;
        return -5;
    }

    // 获取音频流的编解码器参数
    audioCodecParams = formatCtx->streams[asIndex]->codecpar;
    if (!audioCodecParams) {
        cout<<"无法获取编码器参数-音频"<<endl;
        return -6;
    }
     
    // 查找解码器（根据codecParams之中的codec_id编码方式，找到解码器）  //数据 视
    videoCodec = avcodec_find_decoder(videoCodecParams->codec_id);
    if (!videoCodec) {
        cout<<"无法识别解码器-视频"<<endl;
        return -7;
    }

    // 查找解码器（根据codecParams之中的codec_id编码方式，找到解码器）  //数据 音
    audioCodec = avcodec_find_decoder(audioCodecParams->codec_id);
    if (!audioCodec) {
        cout<<"无法识别解码器-音频"<<endl;
        return -8;
    }
         
     
    // 创建解码器上下文  //具体的编码器
    videoCodecCtx = avcodec_alloc_context3(videoCodec);
    if (!videoCodecCtx) {
        cout<<"无法创建上下文信息-视频"<< endl;
        return -9;
    }

    // 创建解码器上下文  //具体的编码器 音
    audioCodecCtx = avcodec_alloc_context3(audioCodec);
    if (!audioCodecCtx) {
        cout<<"无法创建上下文信息-音频"<< endl;
        return -10;
    }
     
    // 将编解码器参数复制到解码器上下文中
    int P2CResultVi = avcodec_parameters_to_context(videoCodecCtx, videoCodecParams);
    if (P2CResultVi<0) {
        cout<<"无法将解码器参数复制到解码器上下文之中-视频"<<endl;
        return -11;
    }

    // 将编解码器参数复制到解码器上下文中
    int P2CResultAu = avcodec_parameters_to_context(audioCodecCtx, audioCodecParams);
    if (P2CResultAu<0) {
        cout<<"无无法将解码器参数复制到解码器上下文之中-音频"<<endl;
        return -12;
    }
     
    // 打开解码器
    int openAvcodecResultVi = avcodec_open2(videoCodecCtx, videoCodec, nullptr);
    if (openAvcodecResultVi<0) {
        cout<<"无法打开解码器-视频"<<endl;
        return -13;
    }

         
    // 打开解码器
    int openAvcodecResultAu = avcodec_open2(audioCodecCtx, audioCodec, nullptr);
    if (openAvcodecResultAu<0) {
        cout<<"无法打开解码器-音频"<<endl;
        return -14;
    }

    //audioCodecCtx正常了
    audioCodecCtxCV.notify_all();

    //用户自定义编码参数  //初始化
    videoEncoder = avcodec_find_encoder_by_name("mpeg4");
    if (!videoEncoder) {
        cout<<"无法找到 mpeg4 编码器，请检查 FFmpeg 是否支持！"<<endl;
        return -15;
    }

    videoEncoderCtx = avcodec_alloc_context3(videoEncoder);
    if (!videoEncoderCtx) {
        cout<<"无法创建编码器上下文信息-视频！"<<endl;
        return -16;
    }
    videoEncoderCtx->profile = FF_PROFILE_H264_HIGH;  // 选择档次
    videoEncoderCtx->width = 1024;                     // 视频宽度
    videoEncoderCtx->height = 436;                     // 视频高度
    videoEncoderCtx->pix_fmt = AV_PIX_FMT_YUV420P;     // MPEG4 标准支持的像素格式
    videoEncoderCtx->time_base = {1, int(25 * playSpeed)}; // 25 FPS
    videoEncoderCtx->bit_rate = 2500000;               // 2.5 Mbps 码率
    videoEncoderCtx->gop_size = 12;                    // 关键帧间隔
    videoEncoderCtx->max_b_frames = 10;                // B帧最大数量
    videoEncoderCtx->level = 5;                        // Level 5

    //打开视频编码器
    int openAvcodecResultViEncoder = avcodec_open2(videoEncoderCtx, videoEncoder, nullptr);
    if (openAvcodecResultViEncoder<0) {
        cout<<"无法打开编码器码器-视频"<<openAvcodecResultViEncoder<<endl;
        avcodec_free_context(&videoEncoderCtx);
        return -17;
    }


    audioEncoder = avcodec_find_encoder_by_name("aac");
    if (!audioEncoder) {
        cout<<"无法找到 aac 编码器！"<<endl;
        return -18;
    }


    // 创建编码器上下文
    audioEncoderCtx = avcodec_alloc_context3(audioEncoder);
    if (!audioEncoderCtx) {
        cout<<"无法创建音频编码器上下文"<<endl;
        return -19;
    }

    // 配置音频参数（需与原始音频匹配或自定义）
    audioEncoderCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;       // AC3常用格式
    audioEncoderCtx->bit_rate = 256000;         // 256kbps 码率
    audioEncoderCtx->sample_rate =  44100;  // 采样率
    audioEncoderCtx->channel_layout = AV_CH_LAYOUT_STEREO;  // 声道布局
    audioEncoderCtx->channels = 2;                          // 声道数
    audioEncoderCtx->time_base = {1, 44100};  // 时间基
    audioEncoderCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;  // 全局头标志
    audioEncoderCtx->frame_size = 1024;


    
    // 打开编码器
    if (avcodec_open2(audioEncoderCtx, audioEncoder, nullptr) < 0) {
        cout<<"无法打开音频编码器！"<<endl;
        return -20;
    }

    return 0;
}


void releaseResources() {
    //通知所有线程读取结束并唤醒等待的线程
    doneVideoReading = true;
    doneAudioReading = true;
    doneVideoFrameReading = true;
    doneVideoFrameEncoderReading = true;
    doneAudioFrameReading = true;       
    doneAudioFrameEncoderReading = true;

    //唤醒所有可能阻塞的条件变量
    videoCV.notify_all();
    audioCV.notify_all();
    videoFrameCV.notify_all();
    videoFrameEncoderCV.notify_all();
    videoEncoderCtxCV.notify_all();
    audioCodecCtxCV.notify_all();
    audioFrameCV.notify_all();           
    audioFrameEncoderCV.notify_all();    
    audioEncoderCtxCV.notify_all();  

    //释放视频数据包队列
    {
        lock_guard<mutex> lock(videoMtx);
        while (!videoPacketQueue.empty()) {
            AVPacket* packet = videoPacketQueue.front();
            av_packet_free(&packet); // 使用 av_packet_free 替代手动 unref 和 delete
            videoPacketQueue.pop();
        }
    }

    //释放音频数据包队列
    {
        lock_guard<mutex> lock(audioMtx);
        while (!audioPacketQueue.empty()) {
            AVPacket* packet = audioPacketQueue.front();
            av_packet_free(&packet);
            audioPacketQueue.pop();
        }
    }

    //释放视频帧队列中的AVFrame
    {
        lock_guard<mutex> lock(videoFrameMtx);
        while (!videoFrameBuffer.empty()) {
            AVFrame* frame = videoFrameBuffer.front();
            av_frame_free(&frame); // 正确释放 AVFrame
            videoFrameBuffer.pop();
        }
    }

    //释放视频编码队列中的AVPacket
    {
        lock_guard<mutex> lock(videoFrameEncoderMtx);
        while (!videoFrameEncoderQueue.empty()) {
            AVPacket* packet = videoFrameEncoderQueue.front();
            av_packet_free(&packet);
            videoFrameEncoderQueue.pop();
        }
    }

    //释放视频解码器上下文和参数
    if (videoCodecCtx) {
        avcodec_free_context(&videoCodecCtx);
        videoCodecCtx = nullptr;
    }
    videoCodecParams = nullptr;

    //释放视频编码器上下文
    {
        lock_guard<mutex> lock(videoEncoderCtxMtx);
        if (videoEncoderCtx) {
            avcodec_free_context(&videoEncoderCtx);
            videoEncoderCtx = nullptr;
        }
    }

    //释放音频解码器上下文和参数
    {
        lock_guard<mutex> lock(audioCodecCtxMtx);
        if (audioCodecCtx) {
            avcodec_free_context(&audioCodecCtx);
            audioCodecCtx = nullptr;
        }
    }
    audioCodecParams = nullptr; 

     //释放音频帧队列
     {
        lock_guard<mutex> lock(audioFrameMtx);
        while (!audioFrameBuffer.empty()) {
            AVFrame* frame = audioFrameBuffer.front();
            av_frame_free(&frame);
            audioFrameBuffer.pop();
        }
    }

    //释放音频编码队列
    {
        lock_guard<mutex> lock(audioFrameEncoderMtx);
        while (!audioFrameEncoderQueue.empty()) {
            AVPacket* packet = audioFrameEncoderQueue.front();
            av_packet_free(&packet);
            audioFrameEncoderQueue.pop();
        }
    }

    //释放 AVFormatContext
    if (formatCtx) {
        avformat_close_input(&formatCtx); //自动释放内部资源并将指针置空
        avformat_free_context(formatCtx);  //确保彻底释放
        formatCtx = nullptr;
    }
}