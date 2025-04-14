#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>       //  

// #include "../log/log.h"
// #include "../buffer/buffer.h"
// #include "httprequest.h"
// #include "httpresponse.h"
#include "log.h"
#include "buffer.h"
#include "httprequest.h"
#include "httpresponse.h"

// 注意哈，这里是综合了前面 httprequest 与 httpresponse
// httprequest 负责解析 请求内容
// httpresponse 负责生成 响应内容
// 这里主要是 相当于将 请求内容 读取到 readBuff_ 之中 然后 httprequest 负责解析 后面 httpresponse 负责生成响应的内容，写到 writeBuff_ 里面 然后再写到 fd 里面
// 这里一定要注意还是值提供接口哈，值提供接口，整体的业务逻辑在最后的webserver，他负责调用
// 注意process函数，他是个什么作用？相当于请求过来了，然后整理一下，生成响应头以及相应的响应文件，他不负责返回给客户端，只负责解析请求然后生成响应
// 这里要注意 ET和LT 这里就不详细解释了，一会到IO多路复用的时候再详细解释 简单的来解释
// --相当于通知我们有数据来了
// -----LT模式下如果缓冲区里面有数据就会一直不停的通知，一直到咱们把缓冲区里面的数据全部读完才会停止通知。因此LT模式下支持分次读或者写，因为如果咱们一次没读完，后面还会不停的继续通知咱们。这样虽然有效，但是确实会浪费系统资源，因为要不停通知
// -----ET模式下只有缓冲区里面来数据了才会通知一次，也只会通知一次。因此，咱们在进行读写的时候必须要保证一次性读完才可以。


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
