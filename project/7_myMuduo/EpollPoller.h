#ifndef EPOLLPOLLER_H
#define EPOLLPOLLER_H

#include "Poller.h"
#include <vector>
#include <sys/epoll.h>


class Channel;

/**
 * epoll的使用  
 * epoll_create
 * epoll_ctl   add/mod/del
 * epoll_wait
 */ 
class EpollPoller : public Poller {
public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller() override;

    // 重写基类Poller的抽象方法
    Timestamp poll(int timeoutMs,ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;
private: 
    static const int kInitEventListSize = 16;
    
    // 填充活跃链接
    void fillActiveChannels(int numEvents,ChannelList *activeChannels) const;
    // 更新channel通道
    void update(int operation,Channel *channel);
    
    using EventList = std::vector<epoll_event>;

    int epollfd_;
    EventList events_;

};

#endif