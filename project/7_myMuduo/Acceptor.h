#ifndef ACCEPTOR_H
#define ACCEPTOR_H
#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

#include <functional>

class EventLoop;
class InetAddress;

class Acceptor : noncopyable
{
public:
    //新连接的回调
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();

    //设置回调
    void setNewConnectionCallback(const NewConnectionCallback &cb) {
        newConnectionCallback_ = cb;
    }

    bool listenning() const { 
        return listenning_; 
    }
    
    void listen();
private:
    //这里不是真正的Read，acceptChannel_会注册到mainloop上，当有新连接来的时候就会唤醒来执行回调，这也是Read事件，这个里面调用的是newConnectionCallback_
    void handleRead();
    
    EventLoop *loop_; // Acceptor用的就是用户定义的那个baseLoop，也称作mainLoop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
};

#endif