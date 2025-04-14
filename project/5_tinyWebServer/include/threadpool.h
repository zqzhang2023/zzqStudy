#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <assert.h>

// 线程池的概念

class ThreadPool {
private:
    // 用一个结构体封装起来，方便调用  其实我感觉没必要，CLASS 已经有了很好的封装性了 再说了，你一个私有的，你让谁调用，你的友元吗
    struct Pool{
        std::mutex mtx_;
        std::condition_variable cond_;
        bool isClosed;
        std::queue<std::function<void()>> tasks;  // 任务队列，函数类型为void()
    };
    std::shared_ptr<Pool> pool_; //这个意思就是用智能指针管理Pool然后咱们不用手动释放了
public:
    // 默认构造函数（C++11特性）
    ThreadPool() = default;
    // 移动构造函数（默认实现）
    ThreadPool(ThreadPool&&) = default;
    /* 主要构造函数
     * @param threadCount: 线程数量，默认为8
     * @note 使用make_shared创建共享资源池，保证内存连续性
     */
    // 尽量用make_shared代替new，如果通过new再传递给shared_ptr，内存是不连续的，会造成内存碎片化
    explicit ThreadPool(int threadCount = 8) : pool_(std::make_shared<Pool>()) { // make_shared:传递右值，功能是在动态内存中分配一个对象并初始化它，返回指向此对象的shared_ptr
        assert(threadCount > 0);                  // 校验线程数量合法性
        // 创建指定数量的工作线程
        for(int i=0;i<threadCount;i++){
            std::thread([this]{ //注意哈，这里一定要把this传进去
                std::unique_lock<std::mutex> locker(pool_->mtx_);    // 自动加锁
                while(true){
                    // 任务队列非空时处理任务
                    if(!pool_->tasks.empty()){
                        // 获取队列首个任务（使用移动语义转移所有权） 左值变右值,资产转移
                        auto task = std::move(pool_->tasks.front());    
                        pool_->tasks.pop(); // 移除队列元素
                        locker.unlock();    // 因为已经把任务取出来了，所以可以提前解锁了
                        task();             // 执行任务
                        locker.lock();      // 马上又要取任务了，上锁
                    }else if(pool_->isClosed){ // 池关闭时退出线程
                        break;
                    }else{  // 队列空时进入等待状态
                        pool_->cond_.wait(locker); // 等待,如果任务来了就notify的
                    }
                }
            }).detach(); // 分离线程（线程生命周期与主线程无关）  其实更好的操作是用一个vector来管理线程
        }
    }

    /* 析构函数 */
    ~ThreadPool() {
        if(pool_){
            std::unique_lock<std::mutex> locker(pool_->mtx_);
            pool_->isClosed = true; // 设置关闭标志
        }
        pool_->cond_.notify_all();  // 唤醒所有的线程
    }


    /* 添加任务到队列
     * @tparam T: 可调用对象类型（自动推导）
     * @param task: 要添加的任务（通用引用）
     */
    template<typename T>
    void AddTask(T&& task){
        std::unique_lock<std::mutex> locker(pool_->mtx_); //上锁
        // 使用完美转发保持参数的值类别
        pool_->tasks.emplace(std::forward<T>(task));
        pool_->cond_.notify_one();                       // 通知一个等待线程
    }
};

#endif