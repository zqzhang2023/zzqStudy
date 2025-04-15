#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>       //  

#include "../log/log.h"
#include "../buffer/buffer.h"
#include "httprequest.h"
#include "httpresponse.h"

class HttpConn {
private:
    // 连接相关成员
    int fd_;                      // 客户端socket文件描述符
    struct sockaddr_in addr_;     // 客户端地址结构
    bool isClose_;                // 连接是否已关闭标记

    // 写操作相关
    int iovCnt_;                  // iov数组有效元素个数
    struct iovec iov_[2];         // 分散写结构数组
    Buffer readBuff_;             // 读缓冲区    读缓冲区（存储客户端请求数据）
    Buffer writeBuff_;            // 写缓冲区    写缓冲区（存储生成的响应头）

    // HTTP处理对象
    HttpRequest request_;         // HTTP请求解析器
    HttpResponse response_;       // HTTP响应生成器

public:
    // 构造函数 & 析构函数
    HttpConn();
    ~HttpConn();

    // 核心接口方法
    void init(int sockFd, const sockaddr_in& addr); // 初始化连接
    ssize_t read(int* saveErrno);                   // 从socket读取数据
    ssize_t write(int* saveErrno);                  // 向socket写入数据
    void Close();                                   // 关闭连接
    int GetFd() const;                              // 获取文件描述符
    int GetPort() const;                            // 获取客户端端口
    const char* GetIP() const;                      // 获取客户端IP字符串
    sockaddr_in GetAddr() const;                    // 获取客户端地址结构
    bool process();                                 // 处理HTTP请求生成响应

    // 写的总长度              // 计算待发送数据总长度
    int ToWriteBytes() { 
        return iov_[0].iov_len + iov_[1].iov_len; 
    }
    
    // 判断是否保持连接
    bool IsKeepAlive() const{
        return request_.IsKeepAlive();
    }

    // 静态成员（类级别共享）
    static bool isET;                    // 标记是否使用ET（边沿触发）模式
    static const char* srcDir;           // 服务器资源目录路径
    static std::atomic<int> userCount;   // 原子计数器，记录当前连接数
};

#endif //HTTP_CONN_H
