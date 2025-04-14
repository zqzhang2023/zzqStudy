#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h> //epoll_ctl()
#include <unistd.h> // close()
#include <assert.h> // close()
#include <vector>
#include <errno.h>

//这个其实没什么好说的，就是对epoll一些接口的封装

class Epoller {
public:
    // 构造函数：创建 epoll 实例并初始化事件数组
    // 参数 maxEvent: 预分配的事件数组大小（默认 1024）
    explicit Epoller(int maxEvent = 1024);
    // 析构函数：关闭 epoll 文件描述符
    ~Epoller();

    // 添加文件描述符到 epoll 实例的监控列表中
    // 参数 fd: 要监控的文件描述符
    // 参数 events: 监控的事件类型（如 EPOLLIN、EPOLLOUT、EPOLLET 等）
    // 返回值: 操作是否成功（true 成功，false 失败）
    bool AddFd(int fd, uint32_t events);

    // 修改已监控文件描述符的事件类型
    // 参数 fd: 已监控的文件描述符
    // 参数 events: 新的事件类型
    // 返回值: 操作是否成功
    bool ModFd(int fd, uint32_t events);

    // 从 epoll 实例中删除文件描述符
    // 参数 fd: 要删除的文件描述符
    // 返回值: 操作是否成功
    bool DelFd(int fd);

    // 等待事件就绪（核心函数）
    // 参数 timeoutMs: 超时时间（毫秒，-1 表示阻塞等待）
    // 返回值: 就绪的事件数量（出错时返回 -1）
    int Wait(int timeoutMs = -1);

    // 获取第 i 个就绪事件对应的文件描述符
    // 参数 i: 事件数组的索引
    // 返回值: 文件描述符（需确保 i 在有效范围内）
    int GetEventFd(size_t i) const;

    // 获取第 i 个就绪事件的事件类型
    // 参数 i: 事件数组的索引
    // 返回值: 事件类型（如 EPOLLIN、EPOLLOUT 等）
    uint32_t GetEvents(size_t i) const;
        
private:
    int epollFd_;   // epoll 实例的文件描述符
    std::vector<struct epoll_event> events_;   // 存储就绪事件的数组 
};


#endif //EPOLLER_H