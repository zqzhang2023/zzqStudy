#include "../include/manageAll.h"

// 全局滤镜相关变量
AVFilterGraph* filter_graph = nullptr;
AVFilterContext* buffersrc_ctx = nullptr;
AVFilterContext* buffersink_ctx = nullptr;

int init_video_filters() {
    filter_graph = avfilter_graph_alloc();
    if (!filter_graph) return AVERROR(ENOMEM);

    // 创建输入buffer滤镜
    char args[512];
    snprintf(args, sizeof(args),
            "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d",
            videoCodecCtx->width,
            videoCodecCtx->height,
            videoCodecCtx->pix_fmt,
            1,25);

    int ret = avfilter_graph_create_filter(&buffersrc_ctx,
                                        avfilter_get_by_name("buffer"),
                                        "in", args, NULL, filter_graph);
    if (ret < 0) {
        avfilter_graph_free(&filter_graph);
        return ret;
    }

    // 创建输出buffersink滤镜
    ret = avfilter_graph_create_filter(&buffersink_ctx,
                                     avfilter_get_by_name("buffersink"),
                                     "out", NULL, NULL, filter_graph);
    if (ret < 0) {
        avfilter_graph_free(&filter_graph);
        return ret;
    }

    // 设置输出像素格式
    enum AVPixelFormat pix_fmts[] = { videoCodecCtx->pix_fmt, AV_PIX_FMT_NONE };
    ret = av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE,
                            AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        avfilter_graph_free(&filter_graph);
        return ret;
    }

    // 解析滤镜描述
    const char* filter_descr = "movie=../resources/watermark.png[wm];[in][wm]overlay=W-w-10:10";
    AVFilterInOut* outputs = avfilter_inout_alloc();
    AVFilterInOut* inputs = avfilter_inout_alloc();
    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_descr,
                                      &inputs, &outputs, NULL)) < 0) {
        avfilter_graph_free(&filter_graph);
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);
        return ret;
    }

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0) {
        avfilter_graph_free(&filter_graph);
        return ret;
    }

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    return 0;
}


void processVideoPackets() {

    bool filters_initialized = false;

    // 初始化滤镜
    if (init_video_filters() == 0) {
        filters_initialized = true;
    } else {
        cerr << "无法初始化滤镜" << endl;
    }
    
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

                    if (filters_initialized) {
                        AVFrame* processed_frame = frame;
                        if (av_buffersrc_add_frame(buffersrc_ctx, frame) < 0) {
                            av_frame_unref(frame);
                            continue;
                        }

                        AVFrame* filtered_frame = av_frame_alloc();
                        while (av_buffersink_get_frame(buffersink_ctx, filtered_frame) >= 0) {
                            processed_frame = filtered_frame;
                            
                            // 将处理后的帧加入缓冲区
                            AVFrame* frameCopy = av_frame_alloc();
                            if (av_frame_ref(frameCopy, processed_frame) == 0) {
                                unique_lock<mutex> frameLock(videoFrameMtx);
                                videoFrameCV.wait(frameLock, [&] { return !videoFrameBuffer.full(); });
                                videoFrameBuffer.push(frameCopy);
                                videoFrameCV.notify_all();
                            }
                            av_frame_unref(processed_frame);
                        }
                        av_frame_free(&filtered_frame);
                    }else{
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