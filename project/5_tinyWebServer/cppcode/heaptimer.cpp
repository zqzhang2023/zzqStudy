#include "../include/heaptimer.h"

// 交换两个节点的位置
void HeapTimer::SwapNode_(size_t i, size_t j) {
    assert(i >= 0 && i <heap_.size());
    assert(j >= 0 && j <heap_.size());
    swap(heap_[i], heap_[j]);
    // 更新交换后的id索引映射
    ref_[heap_[i].id] = i;    // 结点内部id所在索引位置也要变化
    ref_[heap_[j].id] = j;    
}

// 上浮调整
void HeapTimer::siftup_(size_t i){
    assert(i >= 0 && i < heap_.size());
    int parent = (i-1)/2;  // 父节点下标计算
    while(parent >= 0){
        if(heap_[parent] > heap_[i]){  //如果父节点比较大，那么就交换一下，让现在的节点上浮
            SwapNode_(i, parent);
            i = parent;
            parent = (i-1)/2;          //继续判断需不需要上浮动
        }else{
            break;
        }
    }
}

// 下沉调整
// false：不需要下滑  true：下滑成功
bool HeapTimer::siftdown_(size_t i, size_t n) {
    assert(i >= 0 && i < heap_.size());
    assert(n >= 0 && n <= heap_.size());    // n:共几个结点
    auto index = i;
    auto child = 2 * index + 1;             // 左子节点
    while(child < n){
        // 选择较小的子节点 child+1为右子结点
        if(child+1 < n && heap_[child+1] < heap_[child]) {
            child++;//右子结点
        }
        if(heap_[child] < heap_[index]) { //当前节点比子结点要大，则需要下沉
            SwapNode_(index, child);
            index = child;
            child = 2 * child + 1;        //继续看孩子节点
        }else{
            break;  // 需要跳出循环
        }
    }
    return index > i; //说明有下沉，如果index == i 便是没有变动 不需要下沉
}

// 删除指定位置的结点
void HeapTimer::del_(size_t index) {
    assert(index >= 0 && index < heap_.size());
    // 将要删除的结点换到队尾，然后调整堆
    size_t tmp = index;
    size_t n = heap_.size() - 1;
    assert(tmp <= n);
    // 如果就在队尾，就不用移动了,反之就需要交换了
    if(index < heap_.size()-1){
        SwapNode_(tmp, heap_.size()-1);
        // 尝试下沉新节点
        if(!siftdown_(tmp, n)){
            siftup_(tmp); // 若不需要下沉则尝试上浮
        }
    }
    // 删除末尾节点
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

// 调整指定id的结点
void HeapTimer::adjust(int id, int newExpires) {
    assert(!heap_.empty() && ref_.count(id));
    heap_[ref_[id]].expires = Clock::now() + MS(newExpires);
    // 同理，修改完之后要调整堆
    size_t i = ref_[id];
    if(!siftdown_(i, heap_.size())) {
        siftup_(i);  // 若不需要下沉则尝试上浮
    }

}

// 添加新节点或者调整
void HeapTimer::add(int id, int timeOut, const TimeoutCallBack& cb) {
    assert(id >= 0);
    // 如果有，则调整
    if(ref_.count(id)){
        int tmp = ref_[id];
        heap_[tmp].expires = Clock::now() + MS(timeOut);
        heap_[tmp].cb = cb;
        //同理，修改完之后要调整堆
        if(!siftdown_(tmp, heap_.size())) {
            siftup_(tmp);
        }
    }else{
        size_t n = heap_.size();
        ref_[id] = n;
        // 这里应该算是结构体的默认构造
        heap_.push_back({id, Clock::now() + MS(timeOut), cb});  // 右值
        siftup_(n);  // 上浮新节点 因为已经是在最后了，不需要下沉
    }
}

// 删除指定id，并触发回调函数
void HeapTimer::doWork(int id) {
    if(heap_.empty() || ref_.count(id) == 0) { //heap_是空的或者id不存在
        return;
    }
    size_t i = ref_[id];
    auto node = heap_[i];
    node.cb();  // 触发回调函数
    del_(i);    // 删除节点
}

// 清除所有超时节点
void HeapTimer::tick() {
    if(heap_.empty()) {
        return;
    }
    // 循环检查堆顶节点
    while(!heap_.empty()){
        TimerNode node = heap_.front();  //取堆顶节点
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) { 
            break; 
        }   // 未超时则停止
        node.cb(); // 执行超时回调
        pop();     // 删除堆顶
    }
}

// 删除堆顶节点
void HeapTimer::pop() {
    assert(!heap_.empty());
    del_(0);
}

// 清空所有节点
void HeapTimer::clear() {
    ref_.clear();
    heap_.clear();
}

// 获取下一个超时时间
int HeapTimer::GetNextTick() {
    tick();
    int res = -1;
    if(!heap_.empty()) {
        // 计算堆顶节点的剩余时间
        res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if(res < 0) { res = 0; }
    }
    return res;
}