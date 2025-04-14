#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>   //perror
#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <vector>    //readv
#include <atomic>
#include <assert.h>

// 写在前面这里一定要注意buffer的内容，要不然很容易搞迷糊，通过两个指针把buffer分成了三个区域 |---A---|----B----|---C---|
// A区域：左侧是Buffer[0] 右侧是readPos_  这里表示prependable区域(正常情况下应该是读区域和写区域，读区域最开始的index应该是0，数据读除去之后 readPos_++ 就形成了这样一个区域，所以这个区域被称为备用区域，后续可以用这个区域来扩容写空间)
// B区域：左侧是readPos_  右侧是writePos_ 这篇区域表示可以被读出去的区域
// C区域：左侧是writePos_  右侧是readPos_ 这片区域表示便是可以写进来的区域
// 那么我们模拟一下过程，假设一个buffer为1024这么大，那么初始的readPos_，writePos_均为0，现在进行模拟(记得在脑海里面想象一下过程)
// ----现在有一个socketfd要往buffer里面写东西，咱们调用他的read函数，然后把得到东西写入buffer中，假设使用了64字节那么大 (现在就是字节存进取，writePos_后移动64个位置)（现在写区域还剩下1024-64）
// ----服务器读取内容，假设第一次读取32个字节(假设哈，这里是模拟，不是真实的)，那么readPos_向后移动32个位置，这个时候就形成了3个区域了，其中A区域，也就是0-31这些地方的内容已经被读出去了，不再需要了
// ----现在服务器要把返回的东西写进来buffer，假设要写进来的东西是1024-64 + 16 比 还剩下的1024 - 64 多 16个字节，那么咱们就需要查找一下A区域的大小，咱们发现，A区域还有32个字节，可以支持，那么现在就需要合并一下A与C区域
// ---- ----- 合并方法，移动B区域，将B区域移动到最前面，readPos_变成 0 writePos_变成 B.size B区域的大小
// ---- ----- 如果发现合并区域之后还是不行的话，那么就要给buffer多分配空间了

class Buffer{
private:
    std::vector<char> buffer_;         // 地层的容器
    std::atomic<std::size_t> readPos_; // 原子类型的读指针(当前的读的位置)
    std::atomic<std::size_t> writePos_;// 原子类型的写指针(当前的写的位置)

    // 获取缓冲区起始地址（非常量）
    char* BeginPtr_();
    // 获取缓冲区起始地址（常量）
    const char* BeginPtr_() const;
    // 扩容或整理缓冲区以提供至少len字节可写空间
    void MakeSpace_(size_t len);

public:
    //构造函数，初始化缓冲区大小及读写位置
    Buffer(int initBuffSize = 1024);
    //默认析构函数
    ~Buffer() = default;
    //获取当前可写空间大小
    size_t WritableBytes() const;
    //获取当前可读数据大小
    size_t ReadableBytes() const;
    //获取预留空间（已读区域）大小
    size_t PrependableBytes() const;
    //返回当前可读数据的起始地址（常量指针）
    const char* Peek() const;
    //确保至少有len字节的可写空间，否则扩容
    void EnsureWriteable(size_t len);
    //移动写指针（数据写入后调用）
    void HasWritten(size_t len);
    //移动读指针（消费len字节数据）
    void Retrieve(size_t len);
    //移动读指针到指定地址（消费到end的数据）
    void RetrieveUntil(const char* end);
    //重置缓冲区（清空数据，读写指针归零）
    void RetrieveAll();
    //取出所有可读数据并转为字符串，清空缓冲区
    std::string RetrieveAllToStr();
    //返回当前写位置常量指针
    const char* BeginWriteConst() const;
    //返回当前写位置指针
    char* BeginWrite();
    //添加字符串到缓冲区
    void Append(const std::string& str);
    //添加字符数组到缓冲区
    void Append(const char* str,size_t len);
    //添加任意类型数据到缓冲区（转换为char*）
    void Append(const void* data, size_t len);
    //添加另一个缓冲区的可读数据到当前缓冲区
    void Append(const Buffer& buff);
    //从文件描述符读取数据到缓冲区（使用readv,并且注意这里是读数据到缓冲区，所以要放入C区域，操作的是writePos_）
    ssize_t ReadFd(int fd, int* Errno);
    //将缓冲区的可读数据写入文件描述符(注意这里是写入文件描述符，因此要读缓冲区，是从B区域读出去，操作的是readPos_)
    ssize_t WriteFd(int fd, int* Errno);
    //注意上面俩参数，非常容易搞混乱
};

#endif
