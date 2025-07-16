#include "Poller.h"
#include <stdlib.h>
// #include "PollPoller.h"
#include "EpollPoller.h"

// 想一下为啥在这里实现，不在Poller里面实现,因为PollPoller 与 EPollPoller 继承于Poller,如果在Poller.cc 里面实现的话就得包含PollPoller.h EPollPoller.h 感觉有一些反过来了
Poller* Poller::newDefaultPoller(EventLoop *loop){
    if (::getenv("MUDUO_USE_POLL")) {
        return nullptr; // 生成poll的实例
    }
    else {
        return new EpollPoller(loop); // 生成epoll的实例
    }
}