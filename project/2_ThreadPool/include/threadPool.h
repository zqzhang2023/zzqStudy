#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>       // 用于存储工作线程
#include <queue>        // 用于存储任务队列
#include <memory>       // 智能指针
#include <thread>       // 线程库
#include <mutex>        // 互斥锁
#include <condition_variable> // 条件变量
#include <future>       // 异步操作（std::future, std::packaged_task）
#include <functional>   // 函数对象
#include <stdexcept>    // 异常类（如 std::runtime_error）

class ThreadPool{
private:
    std::vector<std::thread> workers;        // 工作线程集合
    std::queue<std::function<void()>> tasks; // 任务队列（存储无参void函数）
    std::mutex queue_mutex;                  // 保护任务队列的互斥锁
    std::condition_variable condition;       // 线程间通信的条件变量
    bool stop;                               // 停止标志
public:
    ThreadPool(size_t); // 构造函数，参数为线程数量
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>; // 任务入队方法
    ~ThreadPool(); // 析构函数
};

inline ThreadPool::ThreadPool(size_t threadsNum):stop(false){
    for(size_t i=0; i<threadsNum; i++){
        workers.emplace_back( // 创建线程并绑定到Lambda
            [this]{
                for(;;){      // 无限循环，直到线程池停止
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock,[this]{
                            return this->stop||!this->tasks.empty();
                        });// 等待任务或停止信号（如果任务列表为空或者是stop为true的话，就会一直等待）

                        if(this->stop && this->tasks.empty()){
                           return;   // 停止且无任务时退出线程
                        }
                        task = std::move(this->tasks.front());  //取任务
                        this->tasks.pop();
                    } //这里会自动释放锁
                    task(); //执行任务
                }
            }
        );
    }
}


template <class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&...args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
{   
    // 获取 F 用 Args... 调用后的返回类型。
    using return_type = typename std::result_of<F(Args...)>::type;  //若 F 是 int foo(double), Args 是 double，则 return_type 是 int。
    // 将用户任务转换为 return_type() 签名（无参数，返回 return_type）
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f),std::forward<Args>(args)...)
    );

    // 从 packaged_task 中提取 future，用户可通过 res.get() 获取最终结果。
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if(this->stop){
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        this->tasks.emplace([task]() { (*task)(); });  // 将任务包装为 void() 类型
    }
    // 唤醒一个正在等待的 worker 线程来处理新任务。
    condition.notify_one();

    return res;
}



inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(this->queue_mutex);
        this->stop = true;  // 设置停止标志
    }
    condition.notify_all(); // 唤醒所有线程
    for(std::thread& worker:this->workers){
        worker.join();      // 等待所有线程结束
    }
}

#endif
