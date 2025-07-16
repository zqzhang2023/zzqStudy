#include "EpollPoller.h"
#include "Logger.h"
#include "Channel.h"

#include <errno.h>
#include <unistd.h>
#include <strings.h>


// channel未添加到poller中
const int kNew = -1;          // channel的成员index_ = -1
// channel已添加到poller中
const int kAdded = 1;
// channel从poller中删除
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop* loop)
    :Poller(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize)  // vector<epoll_event>
{
    // 错误处理
    if(epollfd_ < 0){
        LOG_FATAL("epoll_create error:%d \n",errno);
    }
    
}

EpollPoller::~EpollPoller(){
    ::close(epollfd_);
}


// 重写基类Poller的抽象方法 
// EventLoop 调用 poller.poll，  poller.poll通过将epoll_wait之中发生事件的channel写入activeChannels之中来告知EventLoop那些发生了
Timestamp EpollPoller::poll(int timeoutMs,ChannelList *activeChannels){
    // 实际上应该用LOG_DEBUG输出日志更为合理
    LOG_INFO("func=%s => fd total count:%lu \n", __FUNCTION__, channels_.size());

    int numEvents = ::epoll_wait(epollfd_,&*events_.begin(),static_cast<int>(events_.size()),timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());

    if(numEvents > 0){
        LOG_INFO("%d events happened \n", numEvents);
        fillActiveChannels(numEvents,activeChannels);
        if(numEvents == events_.size()){ //所有的event都发生了，那么可能空间不够，需要扩容
            events_.resize(events_.size() * 2);
        }
    }else if(numEvents == 0){ // 超时链
         LOG_DEBUG("%s timeout! \n", __FUNCTION__);
    }else{ //外部中断
        if (saveErrno != EINTR){
            errno = saveErrno;
            LOG_ERROR("EPollPoller::poll() err!");
        }
    }
    return now;
}

// channel update remove => EventLoop updateChannel removeChannel => Poller updateChannel removeChannel
/**
 *            EventLoop  =>   poller.poll  
 *     ChannelList      Poller
 *                     ChannelMap  <fd, channel*>   epollfd
 */ 
void EpollPoller::updateChannel(Channel *channel){

    const int index = channel->index();
    LOG_INFO("func=%s => fd=%d events=%d index=%d \n", __FUNCTION__, channel->fd(), channel->events(), index);
    // channel还没有注册或者是已经删除掉了（kDeleted的时候Poller里面的ChannelMap里面并没有删除掉,调用removeChannel的时候才会在ChannelMap里面删除掉）
    if(index == kNew || index == kDeleted){ 
        if(index == kNew){
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD,channel);
    } else {  // channel已经在poller上注册过了
        int fd = channel->fd();
        if(channel->isNoneEvent()){ //对任何事情都不感兴趣了，那就删除
            update(EPOLL_CTL_DEL,channel);
            channel->set_index(kDeleted);
        }else{
            update(EPOLL_CTL_MOD,channel);
        }
    }
}

// 从poller中删除channel
void EpollPoller::removeChannel(Channel *channel){
    int fd = channel->fd();
    channels_.erase(fd);

    LOG_INFO("func=%s => fd=%d\n", __FUNCTION__, fd);

    int index = channel->index();
    if(index == kAdded){
        update(EPOLL_CTL_DEL,channel);
    }
    channel->set_index(kNew);

}


// 填充活跃链接
void EpollPoller::fillActiveChannels(int numEvents,ChannelList *activeChannels) const{
    for(int i=0;i<numEvents;i++){
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);  // EventLoop就拿到了它的poller给它返回的所有发生事件的channel列表了
    }
}
// 更新channel通道 epoll_ctl add/mod/del
void EpollPoller::update(int operation,Channel *channel){
    epoll_event event;
    bzero(&event,sizeof event);

    int fd = channel->fd();

    event.events = channel->events();
    event.data.fd = fd;
    event.data.ptr = channel;

    if(::epoll_ctl(epollfd_,operation,fd,&event) < 0){
        if(operation == EPOLL_CTL_DEL){
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }else{
            LOG_FATAL("epoll_ctl add/mod error:%d\n", errno);
        }
    }
}