#include "log.h"

// 构造
Log::Log(){
    fp_ = nullptr;           // 初始化文件指针为空
    deque_ = nullptr;        // 初始化阻塞队列为空
    writeThread_ = nullptr;  // 初始化写线程为空
    lineCount_ = 0;          // 初始化行计数器 
    toDay_ = 0;              // 初始化当前日期
    isAsync_ = false;        // 默认同步模式
}

// 析构函数
Log::~Log(){
    while(deque_->empty()){
        deque_->flush();      // 唤醒消费者，处理掉剩下的任务
    }
    deque_->Close();          // 关闭队列
    if(writeThread_ && writeThread_->joinable()){ // 确保线程存在且可join
        writeThread_->join();
    }
    if(fp_){                  // 冲洗文件缓冲区，关闭文件描述符
        lock_guard<mutex> locker(mtx_);
        flush();              // 清空缓冲区中的数据
        fclose(fp_);          // 关闭日志文件
    }
}

// 唤醒阻塞队列消费者，开始写日志 刷新缓冲区
void Log::flush(){
    if (isAsync_){         // 只有异步日志才会用到deque
        deque_->flush();
    }
    fflush(fp_);           // 清空输入缓冲区
}

// 懒汉模式 局部静态变量法（这种方法不需要加锁和解锁操作）
Log* Log::Instance(){
    static Log log;  // C++11保证静态局部变量线程安全
    return &log;
}

// 异步日志的写线程函数
void Log::FlushLogThread(){ 
    Log::Instance()->AsyncWrite_();   // 调用实际写方法
}

// 异步写线程真正的执行函数
void Log::AsyncWrite_(){
    string str = "";
    while(deque_->pop(str)){ // 从队列获取日志字符串 //阻塞的地方在这里 writeThread_ 相当于deque_的消费者，当没有数据的时候会阻塞
        lock_guard<mutex> locker(mtx_);     // 这里的加锁不是针对队列(安全队列会自己加锁)哈，针对的是 fp_
        fputs(str.c_str(), fp_);            // 写入文件
    }
}

// 初始化日志系统
void Log::init(int level, const char* path, const char* suffix, int maxQueCapacity) {
    isOpen_ = true;       // 标记日志系统开启
    level_ = level;       // 设置日志级别
    path_ = path;         // 日志存储路径
    suffix_ = suffix;     // 文件后缀

    if(maxQueCapacity){   // 异步方式 队列容量>0启用异步
        isAsync_ = true;
        if(!deque_){      // 为空则创建一个
            unique_ptr<BlockQueue<std::string>> newQue(new BlockQueue<std::string>);
            // 因为unique_ptr不支持普通的拷贝或赋值操作,所以采用move
            // 将动态申请的内存权给deque，newDeque被释放
            deque_ = move(newQue);
            // 启动写线程
            unique_ptr<thread>  newThread(new thread(FlushLogThread));
            writeThread_ = move(newThread); //注意这里哈，刚开始写的时候是不是有些疑问呀，就是这个时候已经对writeThread_进行初始化了，该线程已经执行了，但是他执行的啥呢？刚开始啥也么没有应当会阻塞的
        }
    }else{
        isAsync_ = false;
    }
    // 重置行计数器
    lineCount_ = 0;     

    // 时间处理（日志文件按日期滚动）
    time_t timer = time(nullptr);
    struct tm *sysTime = localtime(&timer);
    struct tm t = *sysTime;

    // 构建日志文件名
    path_ = path;     //和前面重复了，原作者没注意估计
    suffix_ = suffix;
    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", 
        path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
    toDay_ = t.tm_mday;      // 记录当前日期

    // 文件操作（加锁保证线程安全）
    {
        lock_guard<mutex> locker(mtx_);
        buff_.RetrieveAll();   // 清空缓冲区
        if(fp_) {              // 处理旧文件指针
            flush();
            fclose(fp_);
        }
        fp_ = fopen(fileName, "a"); // 追加模式打开文件
        if(fp_ == nullptr){         // 文件不存在时创建目录
            mkdir(path_, 0777);     // 创建日志目录（rwx权限）
            fp_ = fopen(fileName, "a"); // 生成目录文件（最大权限）
        }
        assert(fp_ != nullptr);  // 确保文件打开成功
    }
}

// 核心写日志方法
void Log::write(int level, const char *format, ...){
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);    // 获取精确时间（微秒级）
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list vaList;                // 可变参数列表

    /* 日志文件滚动判断 */
    // 条件1：日期变化 或 条件2：行数超过限制
    if(toDay_ != t.tm_mday || (lineCount_ && (lineCount_  %  MAX_LINES == 0))){
        unique_lock<mutex> locker(mtx_);
        locker.unlock();              // 临时释放锁，避免阻塞其他线程

        // 这个时候得重新创建一个新的文件了
        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};          // 时间戳部分（yyyy_mm_dd）
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday); // 目前的精准事件

        if(toDay_!=t.tm_mday){  // 事件部匹配
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path_, tail, suffix_);  //为啥要减72？
            toDay_ = t.tm_mday;      // 更新日期记录
            lineCount_ = 0;          // 重置行计数器
        }else{  // 行数超了
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail, (lineCount_  / MAX_LINES), suffix_);
        }

        locker.lock();  // 重新加锁
        flush();        // 确保数据写入当前文件
        fclose(fp_);    
        fp_ = fopen(newFile, "a");  // 创建新日志文件
        assert(fp_ != nullptr);
    }

    // 在buffer内生成一条对应的日志信息
    // 说实话，我说实话，我觉得这里的buff很没必要
    // 这是一条完整info log 2025-04-11 10:58:52.272179 [info] : Verify name:111 pwd:111 先看这个的话可能会好理解一些
    {
        unique_lock<mutex> locker(mtx_);
        lineCount_++; // 行号递增
        int n = snprintf(buff_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
        t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
        t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);  //这是一条完整info log 2025-04-11 10:58:52.272179 [info] : Verify name:111 pwd:111
        buff_.HasWritten(n);           // 移动写指针
        AppendLogLevelTitle_(level);    // 添加日志级别前缀 [info]

        // 处理可变参数 注意哈，这里的可变参数，刚开始可能会不太明白，但是你看一下上面的int n = snprintf(buff_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
        // 如果传递的是 format = "%d-%02d-%02d %02d:%02d:%02d.%06ld " vaList =         t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec
        // 这样是不是就明白了
        va_start(vaList, format);
        int m = vsnprintf(buff_.BeginWrite(), buff_.WritableBytes(), format, vaList);
        va_end(vaList);

        buff_.HasWritten(m);      // 更新缓冲区写位置
        buff_.Append("\n\0", 2);  // 添加换行和结束符

        // 根据模式选择写入方式
        if(isAsync_ && deque_ && !deque_->full()){  // 异步方式（加入阻塞队列中，等待写线程读取日志信息）
            deque_->push_back(buff_.RetrieveAllToStr());
        }else{ // 同步方式（直接向文件中写入日志信息）
            fputs(buff_.Peek(), fp_);   // 同步就直接写入文件
        }
        buff_.RetrieveAll();    // 清空缓冲区
    }
}

// 添加日志等级
void Log::AppendLogLevelTitle_(int level) {
    switch(level) { // 不同级别添加不同前缀
    case 0:
        buff_.Append("[debug]: ", 9);
        break;
    case 1:
        buff_.Append("[info] : ", 9);
        break;
    case 2:
        buff_.Append("[warn] : ", 9);
        break;
    case 3:
        buff_.Append("[error]: ", 9);
        break;
    default:
        buff_.Append("[info] : ", 9);
        break;
    }
}

// 获取当前日志级别
int Log::GetLevel() {
    lock_guard<mutex> locker(mtx_);
    return level_;
}

// 设置日志级别
void Log::SetLevel(int level) {
    lock_guard<mutex> locker(mtx_);
    level_ = level;
}