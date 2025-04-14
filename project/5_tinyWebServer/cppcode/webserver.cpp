#include "../include/webserver.h"

using namespace std;

// 构造函数实现
WebServer::WebServer(
    int port, int trigMode, int timeoutMS,
    int sqlPort, const char* sqlUser, const  char* sqlPwd,
    const char* dbName, int connPoolNum, int threadNum,
    bool openLog, int logLevel, int logQueSize):
    port_(port), timeoutMS_(timeoutMS), isClose_(false),
    timer_(new HeapTimer()), threadpool_(new ThreadPool(threadNum)), epoller_(new Epoller())
{
    // 是否打开日志标志
    if(openLog) {
        Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
        if(isClose_) { 
            LOG_ERROR("========== Server init error!=========="); 
        }else{
            LOG_INFO("========== Server init ==========");
            // 日志输出当前配置
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (listenEvent_ & EPOLLET ? "ET": "LT"),
                            (connEvent_ & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }

    // 获取当前工作目录作为资源目录
    srcDir_ = getcwd(nullptr, 256);  // 分配256字节缓冲区
    assert(srcDir_);
    strcat(srcDir_, "/resources/");  // 拼接资源目录路径
    HttpConn::userCount = 0;         // 初始化用户连接数
    HttpConn::srcDir = srcDir_;      // 设置静态资源目录


    // 初始化数据库连接池
    SqlConnPool::Instance()->Init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);  // 连接池单例的初始化
    // 初始化事件和初始化socket(监听)
    InitEventMode_(trigMode);
    if(!InitSocket_()) { isClose_ = true;}
}

// 析构函数
WebServer::~WebServer() {
    close(listenFd_);  // 关闭监听socket
    isClose_ = true;
    free(srcDir_);     // 释放资源目录内存
    SqlConnPool::Instance()->ClosePool(); // 关闭数据库连接池
}

// 初始化事件触发模式
void WebServer::InitEventMode_(int trigMode) {
    listenEvent_ = EPOLLRDHUP;                  // 检测socket关闭 // 默认监听事件带断开检测
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;     // EPOLLONESHOT由一个线程处理

    // 根据trigMode参数设置ET模式
    switch (trigMode)
    {
    case 0:   // 全LT模式
        break;
    case 1:   // CONN ET
        connEvent_ |= EPOLLET;
        break;
    case 2:   // LISTEN ET
        listenEvent_ |= EPOLLET;
        break;
    case 3:   // 全ET模式
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    default:   // 默认全ET
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    }
    HttpConn::isET = (connEvent_ & EPOLLET);   // 设置HTTP连接的触发模式
}

// 初始化监听socket
bool WebServer::InitSocket_() {
    int ret;
    // 常规的本地地址信息
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);
    // 套接字创立
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ < 0) {
        LOG_ERROR("Create socket error!", port_);
        return false;
    }

    int optval = 1;
    
    // 端口复用
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        LOG_ERROR("set socket setsockopt error !");
        close(listenFd_);
        return false;
    }

    // 绑定
    ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        LOG_ERROR("Bind Port:%d error!", port_);
        close(listenFd_);
        return false;
    }

    // 监听
    ret = listen(listenFd_, 8);
    if(ret < 0) {
        LOG_ERROR("Listen port:%d error!", port_);
        close(listenFd_);
        return false;
    }

    // 将监听socket加入epoll
    ret = epoller_->AddFd(listenFd_,  listenEvent_ | EPOLLIN);  // 将监听套接字加入epoller 事件是in 也就是传入
    if(ret == 0) {
        LOG_ERROR("Add listen error!");
        close(listenFd_);
        return false;
    }
    SetFdNonblock(listenFd_);           // 非阻塞模式
    LOG_INFO("Server port:%d", port_);
    return true;
}

// 设置非阻塞
int WebServer::SetFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

// 注意这个函数写在这里，稍后看完整体的代码之后要重新回来看一遍这个
void WebServer::Start() {
    int timeMS = -1;  // epoll wait timeout == -1 无事件将阻塞
    if(!isClose_) { LOG_INFO("========== Server start =========="); }
    while(!isClose_){
        // 计算下次定时器触发间隔
        // 注意: GetNextTick 里面会调用 timer_-> tick 哈，这里要注意，我看代码的时候就有这些疑惑
        if(timeoutMS_ > 0) {
            timeMS = timer_->GetNextTick();     // 获取下一次的超时等待事件(至少这个时间才会有用户过期，每次关闭超时连接则需要有新的请求进来)
        }
        // 这里理解了水平触发和边缘触发之后就会很好的理解
        // 注意这里哈 Wait 会等待 timeMS 这么久的时间，如果 时间 超时 说明一个连接过期了，那么就会断开连接，那么就肯定client肯定会发送 断开连接的请求，同理，server肯定会收到 client断开连接的请求
        int eventCnt = epoller_->Wait(timeMS);
        // 处理所有就绪事件
        for(int i = 0; i < eventCnt; i++) {
            int fd = epoller_->GetEventFd(i);
            uint32_t events = epoller_->GetEvents(i);
            if(fd == listenFd_) {  //如果是服务器收到消息，那么一定是来新的连接了
                DealListen_();     //处理连接
            }else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){        // 处理异常事件（断开/错误）
                assert(users_.count(fd) > 0);
                CloseConn_(&users_[fd]);
            }else if(events & EPOLLIN) {   // 读事件
                assert(users_.count(fd) > 0);
                DealRead_(&users_[fd]);
            }else if(events & EPOLLOUT) {  // 写事件
                assert(users_.count(fd) > 0);
                DealWrite_(&users_[fd]);
            }else {
                LOG_ERROR("Unexpected event");
            }
        }

    }
}

// 发送错误信息并关闭连接
void WebServer::SendError_(int fd, const char*info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);  // 直接关闭连接
}

// 关闭连接
void WebServer::CloseConn_(HttpConn* client) {
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd());
    epoller_->DelFd(client->GetFd());  // 从epoll移除
    client->Close();  
}

// 添加新客户端
void WebServer::AddClient_(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].init(fd, addr);  // 初始化HttpConn对象
    // 添加定时器
    if(timeoutMS_ > 0) {
        // 注意这里的bind哈 我们可以看到WebServer::CloseConn_只有一个参数，但是实际上 CloseConn_ 是；类的成员函数
        // 因此他有隐藏的this指针参数 void WebServer::CloseConn_(WebServer* this, HttpConn* client)
        timer_->add(fd, timeoutMS_, std::bind(&WebServer::CloseConn_, this, &users_[fd]));
    }
    epoller_->AddFd(fd, EPOLLIN | connEvent_); // 注册到epoll
    SetFdNonblock(fd);                          // 设置非阻塞
    LOG_INFO("Client[%d] in!", users_[fd].GetFd());
}

// 处理监听套接字，主要逻辑是accept新的套接字，并加入timer和epoller中
// 处理新连接请求（监听socket可读）
void WebServer::DealListen_() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd_, (struct sockaddr *)&addr, &len);
        if(fd <= 0) { return;}
        else if(HttpConn::userCount >= MAX_FD) {
            SendError_(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        AddClient_(fd, addr); // 添加新客户端
    } while(listenEvent_ & EPOLLET);  // ET模式一定要保证一直循环，一直到全部处理结束
}

// 线程池读任务处理
void WebServer::OnRead_(HttpConn* client) {
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);         // 读取客户端套接字的数据，读到httpconn的读缓存区
    if(ret <= 0 && readErrno != EAGAIN) {   // 读异常就关闭客户端
        CloseConn_(client);
        return;
    }
    // 业务逻辑的处理（先读后处理）//注意哈，这里提前调用了client->read已经把所有的请求内容都写道了client对应的读缓冲区了哈，这里一定要注意，很容易混乱
    OnProcess(client);
}

// 处理读事件，主要逻辑是将OnRead加入线程池的任务队列中
void WebServer::DealRead_(HttpConn* client) {
    assert(client);
    ExtentTime_(client);
    threadpool_->AddTask(std::bind(&WebServer::OnRead_, this, client)); // 这是一个右值，bind将参数和函数绑定
}

/* 处理读（请求）数据的函数 */
void WebServer::OnProcess(HttpConn* client) {
    // 首先调用process()进行逻辑处理 //注意哈，这里 process 负责解析请求然后生成响应内容，只有他正常的生成响应内容才能写
    if(client->process()) { // 根据返回的信息重新将fd置为EPOLLOUT（写）或EPOLLIN（读）
        //读完事件就跟内核说可以写了
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
    } else {
        //错误的话就继续监听读，因为之前读的时候错误了process没做好处理，没法写
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLIN);
    }
}

// 线程池写任务处理
void WebServer::OnWrite_(HttpConn* client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno); // 发送数据
    if(client->ToWriteBytes() == 0) { // 发送完了
        // 长连接保持
        if(client->IsKeepAlive()) {
            epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLIN); // 回归换成监测读事件 
            return;
        }
    }else if(ret < 0){
        if(writeErrno == EAGAIN) {  // 缓冲区满了 
            // 缓冲区满，继续监听写事件 继续传输
            epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
            return;
        }
    }
    CloseConn_(client); // 关闭连接 出错了就直接关闭连接
}

// 处理写事件，主要逻辑是将OnWrite加入线程池的任务队列中
void WebServer::DealWrite_(HttpConn* client) {
    assert(client);
    ExtentTime_(client);
    threadpool_->AddTask(std::bind(&WebServer::OnWrite_, this, client));
}

// 刷新客户端超时时间
void WebServer::ExtentTime_(HttpConn* client) {
    assert(client);
    if(timeoutMS_ > 0) { timer_->adjust(client->GetFd(), timeoutMS_); }
}