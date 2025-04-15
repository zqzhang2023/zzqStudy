#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "epoller.h"
#include "../timer/heaptimer.h"

#include "../log/log.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"

#include "../http/httpconn.h"


class WebServer {
public:
    // 构造函数参数说明:
    // port: 服务器监听端口
    // trigMode: 触发模式（0-3对应不同ET/LT组合）
    // timeoutMS: 连接超时时间（毫秒）
    // sqlPort: 数据库端口
    // sqlUser: 数据库用户名
    // sqlPwd: 数据库密码
    // dbName: 数据库名
    // connPoolNum: 数据库连接池数量
    // threadNum: 线程池线程数
    // openLog: 是否开启日志
    // logLevel: 日志等级
    // logQueSize: 日志异步队列大小
    WebServer(
        int port, int trigMode, int timeoutMS, 
        int sqlPort, const char* sqlUser, const  char* sqlPwd, 
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize);
        
    ~WebServer();
    void Start();   // 启动服务器主循环

private:

    bool InitSocket_();                          // 初始化服务器socket
    void InitEventMode_(int trigMode);           // 初始化事件触发模式
    void AddClient_(int fd, sockaddr_in addr);   // 添加新客户端连接

    // 事件处理函数
    void DealListen_();                          // 处理新连接
    void DealWrite_(HttpConn* client);           // 处理写事件
    void DealRead_(HttpConn* client);            // 处理读事件

    // 辅助函数
    void SendError_(int fd, const char*info);    // 发送错误信息
    void ExtentTime_(HttpConn* client);          // 刷新连接超时时间
    void CloseConn_(HttpConn* client);           // 关闭连接

    // 线程池任务函数
    void OnRead_(HttpConn* client);              // 读处理
    void OnWrite_(HttpConn* client);             // 写处理
    void OnProcess(HttpConn* client);            // 业务逻辑处理
    
    // 最大文件描述符限制
    static const int MAX_FD = 65536;
    // 设置文件描述符非阻塞
    static int SetFdNonblock(int fd);


    // 成员变量
    int port_;            // 监听端口
    bool openLinger_;     // 是否开启优雅关闭
    int timeoutMS_;       // 连接超时时间（毫秒）
    bool isClose_;        // 服务器是否关闭标志
    int listenFd_;        // 监听socket文件描述符
    char* srcDir_;         // 资源文件目录路径

    uint32_t listenEvent_;  // 监听事件HttpConn
    uint32_t connEvent_;    // 连接事件
   
    std::unique_ptr<HeapTimer> timer_;        // 定时器（堆实现）
    std::unique_ptr<ThreadPool> threadpool_;  // 线程池
    std::unique_ptr<Epoller> epoller_;        // Epoll实例
    std::unordered_map<int, HttpConn> users_; // 客户端连接映射表（fd到HttpConn）
};

#endif //WEBSERVER_H
