#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <deque>
#include <condition_variable>
#include <mutex>
#include <sys/time.h>
using namespace std;

// 这是一个安全队列，咱们普通的队列在进行多线程操作的时候会出现互斥，必须得先加锁，再进行队列操作才可以，现在将这些加锁的操作封装到队列之中，这就是安全队列
// 在其他多线程的地方调用安全队列的方法的的时候不需要加锁，因为内部已经加锁了

template<typename T>
class BlockQueue{
private:
    deque<T> deq_;                      // 底层双端队列容器
    mutex mtx_;                         // 互斥锁，保护队列操作
    bool isClose_;                      // 关闭标志
    size_t capacity_;                   // 队列关闭标志
    condition_variable condConsumer_;   // 消费者条件变量（队列空时等待）
    condition_variable condProducer_;   // 生产者条件变量（队列满时等待）
public:
    explicit BlockQueue(size_t maxsize = 1000);   // 构造函数，设置队列容量
    ~BlockQueue();                                // 析构函数，关闭队列并释放资源
    bool empty();                                 // 判空
    bool full();                                  // 判满
    void push_back(const T& item);                // 在队尾添加元素
    void push_front(const T& item);               // 在队头添加元素
    bool pop(T& item);                            // 弹出的任务放入item
    bool pop(T& item, int timeout);               // 带超时的弹出操作
    void clear();                                 // 清空队列
    T front();                                    // 获取队头元素
    T back();                                     // 获取队尾元素
    size_t capacity();                            // 获取队列容量
    size_t size();                                // 获取当前元素数量

    void flush();                                 // 唤醒一个消费者线程
    void Close();                                 // 关闭队列，唤醒所有线程

};

/* 构造函数
 * @param maxsize 队列最大容量，必须大于0
 */
template<typename T>
BlockQueue<T>::BlockQueue(size_t maxsize):capacity_(maxsize){
    assert(maxsize > 0);        // 确保容量合法
    isClose_ = false;           // 初始化关闭标志为false
}

/* 析构函数：调用Close清理资源 */
template<typename T>
BlockQueue<T>::~BlockQueue(){  // 析构函数：调用Close清理资源
    Close();
}

/* 清空队列 */
template<typename T>
void BlockQueue<T>::clear(){
    lock_guard<mutex> locker(mtx_);  // 线程安全，一定要加锁
    deq_.clear();
}

/* 关闭队列：清空队列并唤醒所有等待线程 */
template<typename T>
void BlockQueue<T>::Close(){
    clear();  //内部加锁了，咱们这里就不用加锁了
    isClose_ = true;
    // 唤醒所有等待的消费者和生产者线程
    condConsumer_.notify_all();
    condProducer_.notify_all();
}

/* 判断队列是否为空 */
template<typename T>
bool BlockQueue<T>::empty(){
    lock_guard<mutex> locker(mtx_);  // 线程安全，一定要加锁
    return deq_.empty();
}

/* 判断队列是否已满 */
template<typename T>
bool BlockQueue<T>::full(){
    lock_guard<mutex> locker(mtx_);  // 线程安全，一定要加锁
    return deq_.size() >= capacity_;// 当前大小 >= 容量即为满
}

/* 在队尾插入元素（生产者调用）*/
template<typename T>
void BlockQueue<T>::push_back(const T& item){
    // 注意，条件变量需要搭配unique_lock
    unique_lock<mutex> locker(mtx_);
    condProducer_.wait(locker,[this]{
        return !(this->deq_.size() >= this->capacity_)||this->isClose_;
    });
    // 检查队列是否已关闭
    if (isClose_) { //关闭就抛出异常
        return;
    }
    deq_.push_back(item);               // 插入元素到队尾
    condConsumer_.notify_one();         // 通知一个消费者可以取数据
}

/* 在队头插入元素（生产者调用）*/
template<typename T>
void BlockQueue<T>::push_front(const T& item){
    // 注意，条件变量需要搭配unique_lock
    unique_lock<mutex> locker(mtx_);
    condProducer_.wait(locker,[this]{
        return !(this->deq_.size() >= this->capacity_)||this->isClose_;
    });
    // 检查队列是否已关闭
    if (isClose_) { //关闭就return
        return;
    }
    deq_.push_front(item);               // 插入元素到队尾
    condConsumer_.notify_one();         // 通知一个消费者可以取数据
}

/* 弹出队头元素（消费者调用，阻塞）*/
template<typename T>
bool BlockQueue<T>::pop(T& item) {
    unique_lock<mutex> locker(mtx_);    // 必须使用unique_lock以便操作条件变量
    while(deq_.empty()) {               // 队列空则等待
        condConsumer_.wait(locker);     // 挂起消费者，等待队列非空信号
        if(isClose_){                   // 检查队列是否已关闭
            return false;
        }
    }
    item = deq_.front();                // 取队头元素
    deq_.pop_front();
    condProducer_.notify_one();         // 通知生产者有空位
    return true;
}

/* 弹出队头元素（支持超时等待）
 * @param timeout 超时时间（秒）
 * @return 超时或队列关闭返回false，否则true
 */
template<typename T>
bool BlockQueue<T>::pop(T &item, int timeout) {
    unique_lock<std::mutex> locker(mtx_);   // 必须使用unique_lock以便操作条件变量
    while(deq_.empty()){
        if(condConsumer_.wait_for(locker, std::chrono::seconds(timeout)) 
                == std::cv_status::timeout){
            return false;                  // 超时返回false
        }
        if(isClose_){                      // 检查队列是否已关闭
            return false;
        }
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}


/* 获取队头元素（线程安全）*/
template<typename T>
T BlockQueue<T>::front() {
    lock_guard<std::mutex> locker(mtx_);
    return deq_.front();
}

/* 获取队尾元素（线程安全）*/
template<typename T>
T BlockQueue<T>::back() {
    lock_guard<std::mutex> locker(mtx_);
    return deq_.back();
}

/* 获取队列容量 */
template<typename T>
size_t BlockQueue<T>::capacity() {
    lock_guard<std::mutex> locker(mtx_);
    return capacity_;
}

/* 获取当前元素数量 */
template<typename T>
size_t BlockQueue<T>::size() {
    lock_guard<std::mutex> locker(mtx_);
    return deq_.size();
}

/* 唤醒一个消费者线程（常用于手动触发处理）*/
template<typename T>
void BlockQueue<T>::flush() {
    condConsumer_.notify_one();
}

#endif