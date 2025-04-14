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
#include "buffer.h"

// 写在前面
// 日志系统对整个项目来说非常重要，涉及到一些错误调试操作
// 1.单例模式：确保整个程序中只有一个日志实例，在这样一个“庞大”的系统之中，无法避免的的要全局的访问日志系统，如果使用多个日志实例的话，那么就需要考虑很多很多的同步的问题，因此单例模式是最优解
// -- 这里要注意，这个项目是C++11实现的 C++11标准 （§6.7 [stmt.dcl] 第4段） 如果控制流在变量初始化时并发进入声明，并发线程必须等待初始化完成。 也就是说 静态局部变量 是线程安全的
// 2.同步与异步：这里又涉及到了同步与异步的概念，这个可以搜索了解一下，这里简单来说，就是：同步，在写入文件的时候直接在主线程里面写；异步：重新申请一个线程，在线程里面写，不会阻塞主线程
// --咱们来理一下整个流程
// --1.向外接口是 LOG_INFO （以LOG_INFO为例哈，其他的还有LOG_DEBUG啥的）看下面的宏定义。首先会获取实例，然后经过判断调用write
// --2.在写的时候先判断一下对应的文件(比如日期，比如文件的行数是不是超过最大的行数)，如果有问题的话会重新生成一个新的日志文件
// --3.然后会组合日志的语句，存储到buffer之中。比如：一行日志：2025-04-11 10:58:46.397812 [info] : Client[23](127.0.0.1:59563) in, userCount:6 这里需要生成前面的2025-04-11 10:58:46.397812 [info]还要组合后面的内容
// --4.最后才是写进去，这里要注意，写的时候就涉及到同步和异步了
// -------同步：直接在主线程里面写就行了，也就是说直接写，并不需要啥多余的操作，因此这里的IO操作就会对主线程造成阻塞
// -------异步：通过申请一个新的线程来写。具体操作：设置一个阻塞队列，典型的生产者-消费者模型。咱们申请的线程会一直读取这个队列里面的的内容(也就是pop)，如果为空的话就会线程就会阻塞，但是只要有值push进去的话，就会重新唤醒线程
//--------     因此，在异步的情况下，直接将buffer之中的内容push到队列之中就行了，push之后会唤醒进程从而执行写的操作
// 3.还有一个小点，就是可变参数，这个不解释其实也能的懂，但是这边还是记录一下把
// -- 正常的调用比如：LOG_INFO("11111") 这种是没有可变参数的
// -- 带有可变参数的比如：LOG_INFO("%s,%s,%d",str1,str2,int1) 这样就是有可变参数的
// -- 这里主要是容易搞混 
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