#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include "noncopyable.h"

// 定义日志的级别 INFO ERROR FATAL DEBUG
enum LogLevel{
    INFO,  // 普通信息
    ERROR, // 错误信息
    FATAL, // 崩溃信息(无法挽回的一些错误)
    DEBUG, // 调试信息
};

// 输出一个日志类 //不需要进行拷贝构造和赋值
class Logger : noncopyable {
public:
    // 获取日志唯一的实例对象
    static Logger& instance();
    // 设置日志级别
    void  setLogLevel(int level);
    // 写日志
    void log(std::string message);
private:
    int logLevel_; //日志级别
    // 构造函数私有化 单例模式
    Logger(){}
};
// 因为在用户使用的时候不可能初始化日志，设置日志的level等操作，因此定义好提供给用户使用的宏
// LOG_INFO("%s,%d",ard1,arg2)
#define LOG_INFO(logMsgFormat,...)\
    do \
    {  \
        Logger& logger = Logger::instance();\
        logger.setLogLevel(INFO);\
        char buf[1024] = {0};\
        snprintf(buf,1024,logMsgFormat,##__VA_ARGS__);\
        logger.log(buf);\
    }while(0)

#define LOG_ERROR(logMsgFormat,...)\
    do \
    {  \
        Logger& logger = Logger::instance();\
        logger.setLogLevel(ERROR);\
        char buf[1024] = {0};\
        snprintf(buf,1024,logMsgFormat,##__VA_ARGS__);\
        logger.log(buf);\
    }while(0)

#define LOG_FATAL(logMsgFormat,...)\
    do \
    {  \
        Logger& logger = Logger::instance();\
        logger.setLogLevel(FATAL);\
        char buf[1024] = {0};\
        snprintf(buf,1024,logMsgFormat,##__VA_ARGS__);\
        logger.log(buf);\
        exit(-1);\
    }while(0)

// DEBUG的信息比较多，而且一般不提供给用户使用，因此设置一个MUDEBUG开关来操作一下
#ifdef MUDEBUG
#define LOG_DEBUG(logMsgFormat,...)\
    do \
    {  \
        Logger& logger = Logger::instance();\
        logger.setLogLevel(DEBUG);\
        char buf[1024] = {0};\
        snprintf(buf,1024,logMsgFormat,##__VA_ARGS__);\
        logger.log(buf);\
    }while(0)
#else
    #define LOG_DEBUG(logMsgFormat,...)
#endif

#endif