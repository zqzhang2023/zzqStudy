#include "Thread.h"
#include "CurrentThread.h"

#include <semaphore.h>

std::atomic_int Thread::numCreated_(0);

Thread::Thread(ThreadFunc func,const std::string &name)
    : started_(false)
    , joined_(false)
    , tid_(0)
    , func_(std::move(func))
    , name_(name)
{
    setDefaultName();
}

Thread::~Thread(){
    if(started_ && !joined_){
        thread_->detach();    // thread类提供的设置分离线程的方法 detach之后等到线程结束之后会自动join
    }
}

void Thread::setDefaultName(){
    int num = ++ numCreated_;
    if(name_.empty()){
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}

void Thread::start(){    // 一个Thread对象，记录的就是一个新线程的详细信息
    started_ = true;
    sem_t sem;
    sem_init(&sem,false,0);  //信号量为0
    // 开启线程
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        // 获取线程的tid值
        tid_ = CurrentThread::tid();
        sem_post(&sem); //信号量+1
        // 开启一个新线程，专门执行该线程函数
        func_(); 
    }));
    // 这里必须等待获取上面新创建的线程的tid值,因为有时候会使用tid是否大于0来判断是否创建成功
    sem_wait(&sem); //信号量-1
}

void Thread::join(){
    joined_ = true;
    thread_->join();
}
