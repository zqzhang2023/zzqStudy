#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"
#include <sys/epoll.h>

const int Channel::kNoneEvent = 0;                   //没有任何事件
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;  //读事件
const int Channel::kWriteEvent = EPOLLOUT;           //写事件

//构造函数
Channel::Channel(EventLoop *loop,int fd)
    :loop_(loop),fd_(fd),events_(0),revents_(0),index_(-1),tied_(false){}

//析构函数
Channel::~Channel(){}

// channel的tie方法什么时候调用过？
void Channel::tie(const std::shared_ptr<void> &obj){
    tie_ = obj;
    tied_ = true;
}

/**
 * 当改变channel所表示fd的events事件后，update负责在poller里面更改fd相应的事件epoll_ctl
 * EventLoop => ChannelList   Poller,  Poller与ChannelList是独立的，因此只能通过EventLoop来操作
 */
void Channel::update(){
    // 通过channel所属的EventLoop，调用poller的相应方法，注册fd的events事件
    loop_->updateChannel(this);
}

// 在channel所属的EventLoop中， 把当前的channel删除掉
void Channel::remove(){
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime){
    if(tied_){
        //weak_ptr不控制对象的生命周期，但是，它知道对象是否还活着
        //用lock()函数把它可以提升为shared_ptr，如果对象还活着，返回有效的shared_ptr，如果对象已经死了，提升会失败，返回一个空的shared_ptr。
        std::shared_ptr<void> guard = tie_.lock();
        if(guard){
            handleEventWithGuard(receiveTime);
        }
    }else{
        handleEventWithGuard(receiveTime);
    }
}

// 根据poller通知的channel发生的具体事件， 由channel负责调用具体的回调操作
// 根据相应的事件来调用相应的回调
void Channel::handleEventWithGuard(Timestamp receiveTime){

    LOG_INFO("channel handleEvent revents:%d\n", revents_);
    // 关闭事件
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if (closeCallback_) {
            closeCallback_();
        }
    }
    // 错误事件
    if (revents_ & EPOLLERR) {
        if (errorCallback_) {
            errorCallback_();
        }
    }
    // 读事件
    if (revents_ & (EPOLLIN | EPOLLPRI)) {
        if (readCallback_) {
            readCallback_(receiveTime);
        }
    }
    // 写事件
    if (revents_ & EPOLLOUT) {
        if (writeCallback_) {
            writeCallback_();
        }
    }
}