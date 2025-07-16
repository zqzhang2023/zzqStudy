#ifndef CHANNEL_H
#define CHANNEL_H

#include "noncopyable.h"
#include "Timestamp.h"
#include <functional>
#include <memory>

//直接前置声明，不包含头文件,直接后面在源文件之中包含头文件，这样的话就可以直接编译成库从而不用提供给用户头文件的信息，防止泄漏更多的东西
class EventLoop; 


/**
 * 理清楚  EventLoop、Channel、Poller之间的关系   《= Reactor模型上对应 Demultiplex
 * Channel 理解为通道，封装可sockfd和其感兴趣的event 如EPOLLIN  EPOLLOUT
 * 还绑定了poller返回的具体事件
 */

class Channel : noncopyable {
public:
    //事件回调
    using EventCallback = std::function<void()>;
    //只读事件回调
    using ReadEventCallback = std::function<void(Timestamp)>; 
    
    //构造函数
    Channel(EventLoop *loop,int fd);
    ~Channel(); //析构函数

    // fd得到poller通知以后，处理事件的函数
    void handleEvent(Timestamp receiveTime);


    // 设置回调函数对象
    void setReadCallback(ReadEventCallback cb) {
        readCallback_ = std::move(cb);
    }  //读事件回调
    void setWriteCallback(EventCallback cb){
        writeCallback_ = std::move(cb);
    }  //写事件回调
    void setCloseCallback(EventCallback cb){
        closeCallback_ = std::move(cb);
    }  //关闭事件回调
    void setErrorCallback(EventCallback cb){
        errorCallback_ = std::move(cb);
    }  //错误事件回调

    // 防止当channel被手动remove掉，channel还在执行回调，使用弱智能指针实现
    void tie(const std::shared_ptr<void> &);

    // 返回绑定的fd
    int fd() const {
        return fd_;
    }
    // 返回注册的事件
    int events() const {
        return events_;
    }
    // 设置 revents_ 真正发生的事件，这个接口是放给epoll和eventloop用的
    void set_revents(int revt){
        revents_ = revt;
    }

    // 设置fd相应的事件的状态
    void enableReading() { 
        events_ |= kReadEvent; 
        update(); 
    } //读事件
    void disableReading() { 
        events_ &= ~kReadEvent; 
        update(); 
    } //取消读
    void enableWriting() { 
        events_ |= kWriteEvent; 
        update(); 
    } //写事件 
    void disableWriting() { 
        events_ &= ~kWriteEvent; 
        update(); 
    } //取消写
    void disableAll() { 
        events_ = kNoneEvent; 
        update(); 
    } //取消所有的事件

    // 返回fd当前的事件状态
    bool isNoneEvent() const {
        return events_ == kNoneEvent;
    } //对所有事件都不敢兴起
    bool isWriting() const {
        return events_ == kWriteEvent;
    } //对写事件感兴趣
    bool isReading() const {
        return events_ == kReadEvent;
    } //对读事件感兴趣

    // 返回当前的状态 -1：channel未添加到poller中 1 channel已添加到poller中 2 channel从poller中删除
    int index() {
        return index_;
    }
    // 设置状态，这个接口也是放给epoll和eventloop用的
    void set_index(int index){
        index_ = index;
    }
    // 一个线程有一个EventLoop，一个EventLoop里面有一个poller，一个poller可以监听很多的channel，因此一个channel需要有一个EventLoop。但是一个EventLoop可以包含很多的channel
    // 这个方法就是返回该channel属于那个EventLoop
    EventLoop* ownerLoop() {
        return loop_;
    }
    // 删除channel用的
    void remove();

private:

    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;  //没有任何事件
    static const int kReadEvent;  //读事件
    static const int kWriteEvent; //写事件

    EventLoop *loop_; // 事件注册
    const int fd_;    // fd,poller监听的对象
    int events_;      // 注册fd感兴趣的事件
    int revents_;     // poller返回的具体发生的事件


    // // index表示什么？ 在 EpollPoller之中右这个
    // // channel未添加到poller中
    // const int kNew = -1;          // channel的成员index_ = -1
    // // channel已添加到poller中
    // const int kAdded = 1;
    // // channel从poller中删除
    // const int kDeleted = 2;
    int index_;

    // weak_ptr 可以转化为 share_ptr 
    // 用lock()函数把它可以提升为shared_ptr，如果对象还活着，返回有效的shared_ptr，如果对象已经死了，提升会失败，返回一个空的shared_ptr。
    std::weak_ptr<void> tie_;
    bool tied_;

    // 因为Channel通道里面能够获取fd最终发生的事件的revents，所以它负责调用具体事件的回调操作
    // 这些方法需要用户传递过来
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};



#endif