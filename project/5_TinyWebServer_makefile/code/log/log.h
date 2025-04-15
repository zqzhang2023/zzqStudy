#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>           // 可变参数支持（va_start, va_end）
#include <assert.h>
#include <sys/stat.h>         // 目录操作（mkdir）
#include "blockqueue.h"
#include "../buffer/buffer.h"

// 写在前面
// 

class Log{
private:
    // 常量定义  static const 可以直接在里面初始化
    static const int LOG_PATH_LEN = 256; // 日志文件最长文件名
    static const int LOG_NAME_LEN = 256; // 日志最长名字
    static const int MAX_LINES = 50000;  // 日志文件内的最长日志条数
    // 日志文件相关参数
    const char* path_;   // 日志文件存储路径
    const char* suffix_; // 日志文件后缀名
    int MAX_LINES_;      // 最大日志行数
    // 运行时状态
    int lineCount_;      // 当前日志文件行数计数器
    int toDay_;          // 记录当前日期（用于日志滚动）
    bool isOpen_;        // 日志系统是否开启标志
    // 缓冲区与日志级别
    Buffer buff_;        // 输出的内容，缓冲区
    int level_;          // 日志等级
    bool isAsync_;       // 是否开启异步日志
    // 文件与线程相关
    FILE* fp_;           // 打开log的文件指针
    std::unique_ptr<BlockQueue<std::string>> deque_; // 阻塞队列（用于异步模式） //注意哈，这里设置的是一个智能指针因此用的时候注意一下，这是一个指针
    std::unique_ptr<std::thread> writeThread_;       // 异步写线程 指针，同队列理
    std::mutex mtx_;                                 // 互斥锁（保证线程安全）
    
    // 私有构造与析构
    Log();                                          // 构造函数（私有，单例模式）
    void AppendLogLevelTitle_(int level);           // 添加日志级别前缀
    virtual ~Log();                                 // 析构函数
    void AsyncWrite_();                             // 异步写日志的实际执行方法


public:
    // 初始化日志系统
    // 参数：level-日志级别, path-日志路径, suffix-日志后缀, maxQueueCapacity-阻塞队列容量
    void init(int level, const char* path = "./log", const char* suffix = ".log",int maxQueueCapacity = 1024);

    static Log* Instance();       // 获取单例实例
    static void FlushLogThread(); // 异步写日志公有方法，调用私有方法asyncWrite 异步写日志线程入口

    // 写日志方法（核心方法
    // 参数：level-日志级别, format-格式化字符串, ...-可变参数
    void write(int level,const char* format,...);  // 将输出内容按照标准格式整理
    void flush();                                  // 刷新缓冲区到文件
    int GetLevel();                                // 获取当前日志级别
    void SetLevel(int level);                      // 设置日志级别
    bool IsOpen() { return isOpen_; }              // 判断日志系统是否开启
};

/******************** 日志宏定义（对外接口） ********************/
// 基础日志宏
#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::Instance();\
        if (log->IsOpen() && log->GetLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__); \
            log->flush();\
        }\
    } while(0);

// 四个宏定义，主要用于不同类型的日志输出，也是外部使用日志的接口
// ...表示可变参数，__VA_ARGS__就是将...的值复制到这里
// 前面加上##的作用是：当可变参数的个数为0时，这里的##可以把把前面多余的","去掉,否则会编译出错。
#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);    
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif //LOG_H