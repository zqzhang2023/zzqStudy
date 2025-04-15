#include "epoller.h"

// 构造函数
// 参数 maxEvent: 预分配事件数组的大小
// 创建 epoll 实例，参数 512 是历史遗留值（内核自动调整） 现在可以使用epoll_create1(0) 来创建
// 初始化事件数组大小为 maxEvent
Epoller::Epoller(int maxEvent):epollFd_(epoll_create(512)), events_(maxEvent){
    // 断言检查：确保 epoll 实例创建成功且事件数组非空
    assert(epollFd_ >= 0 && events_.size() > 0);
}

// 析构函数
Epoller::~Epoller() {
    close(epollFd_);  // 关闭 epoll 文件描述符，释放资源
}

// 添加文件描述符到 epoll 监控列表
bool Epoller::AddFd(int fd, uint32_t events) {
    if(fd < 0) return false;  // 检查文件描述符有效性
    epoll_event ev = {0};     // 初始化事件结构体（清零防止脏数据）
    ev.data.fd = fd;          // 关联目标文件描述符
    ev.events = events;       // 设置监控的事件类型
    // 调用 epoll_ctl 添加监控（EPOLL_CTL_ADD 模式）
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev);
}

// 修改已监控文件描述符的事件类型
bool Epoller::ModFd(int fd, uint32_t events) {
    if(fd < 0) return false;   // 检查文件描述符有效性
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    // 调用 epoll_ctl 修改事件（EPOLL_CTL_MOD 模式）
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev);
}

// 从 epoll 监控列表中删除文件描述符
bool Epoller::DelFd(int fd) {
    if(fd < 0) return false;  // 检查文件描述符有效性
    // 调用 epoll_ctl 删除监控（EPOLL_CTL_DEL 模式，最后一个参数可忽略）
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, 0);
}

// 返回事件数量
int Epoller::Wait(int timeoutMs) {
    // 调用 epoll_wait 获取就绪事件
    // 参数 1: epoll 实例的文件描述符
    // 参数 2: 事件数组首地址（底层数组需连续，vector 保证这一点）
    // 参数 3: 最多返回的事件数（设为数组大小避免溢出）
    // 参数 4: 超时时间（-1 表示无限等待）
    return epoll_wait(epollFd_, &events_[0], static_cast<int>(events_.size()), timeoutMs);
    // 返回值:
    // - 正数: 就绪事件数
    // - 0: 超时无事件
    // - -1: 出错（需检查 errno）
}

// 获取事件的fd // 获取第 i 个事件关联的文件描述符
int Epoller::GetEventFd(size_t i) const {
    // 断言确保索引 i 在合法范围内（注意实际就绪事件数可能小于数组大小）
    assert(i < events_.size() && i >= 0);
    return events_[i].data.fd;
}

// 获取事件属性 // 获取第 i 个事件的事件类型
uint32_t Epoller::GetEvents(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].events;
}