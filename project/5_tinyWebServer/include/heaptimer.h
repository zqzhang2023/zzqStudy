#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>
#include "log.h"


// 这个非常的巧妙
// 先说一下为什么要有这个，有些socket连接很长事件不进行操作，还白白占用一个连接，这就很不合理，因此设置一个计时器，当超过一定时间之后就断开连接
// 通过小顶堆来实现，放在最前面的永远是最早就要超时的
// 通过tick检验超时，并调整


// 定义超时回调函数类型
typedef std::function<void()> TimeoutCallBack;
// 使用高精度时钟
typedef std::chrono::high_resolution_clock Clock;
// 毫秒时间单位
typedef std::chrono::milliseconds MS;
// 时间点类型
typedef Clock::time_point TimeStamp;

// 定时器节点结构体
struct TimerNode {
    int id;             // 节点唯一标识
    TimeStamp expires;  // 超时的时间点
    TimeoutCallBack cb; // 回调function<void()>
    // 重载小于运算符，用于比较两个节点的超时时间
    bool operator<(const TimerNode& t) {    // 重载比较运算符
        return expires < t.expires;
    }
    // 重载大于运算符
    bool operator>(const TimerNode& t) {    // 重载比较运算符
        return expires > t.expires;
    }
};

// 堆定时器类（最小堆实现）
class HeapTimer{
private:
    std::vector<TimerNode> heap_;
    std::unordered_map<int, size_t> ref_;   // id对应的在heap_中的下标，方便用heap_的时候查找

    void del_(size_t i);                    // 删除指定位置的节点
    void siftup_(size_t i);                 // 将节点上浮以维护堆结构
    bool siftdown_(size_t i, size_t n);     // 将节点下沉，返回是否进行了下沉
    void SwapNode_(size_t i, size_t j);     // 交换两个节点的位置
public:
    HeapTimer() { heap_.reserve(64); }      // 预分配空间以减少扩容开销
    ~HeapTimer() { clear(); }

    void adjust(int id, int newExpires);    // 调整指定id节点的超时时间
    void add(int id, int timeOut, const TimeoutCallBack& cb);     // 添加新节点或更新已有节点
    void doWork(int id);                    // 执行指定id节点的回调并删除节点
    void clear();                           // 清空所有节点
    void tick();                            // 心跳，很重要很重要！！！
    void pop();                             // 删除堆顶节点
    int GetNextTick();                      // 获取下一个即将超时节点的剩余时间
};

#endif //HEAP_TIMER_H