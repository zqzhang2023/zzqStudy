#ifndef POLLER_H
#define POLLER_H

#include "noncopyable.h"
#include "Timestamp.h"
#include <vector>
#include <unordered_map>

class Channel;
class EventLoop;

// muduo库中多路事件分发器的核心IO复用模块
class Poller: noncopyable {
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop *loop);    // 构造函数
    virtual ~Poller() = default;// 析构函数

    // 给所有的IO复用保留统一的接口 select poll epoll
    virtual Timestamp poll(int timeoutMs,ChannelList *activeChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;   //updata Channel的事件
    virtual void removeChannel(Channel *channel) = 0;   //删除Channel的
    
    // 判断参数channel是否在poller之中
    bool hasChannel(Channel *channel) const;

    // EventLoop可以通过该接口获取默认的IO复用的具体实现
    static Poller* newDefaultPoller(EventLoop *loop);
protected:
    // map的key：sockfd  value：sockfd所属的channel通道类型
    using ChannelMap = std::unordered_map<int,Channel*>;
    ChannelMap channels_;
private:
    EventLoop *ownerLoop_; //定义Poller所属的事件循环EventLoop
};

#endif