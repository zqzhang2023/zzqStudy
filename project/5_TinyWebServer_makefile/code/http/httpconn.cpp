#include "httpconn.h"
using namespace std;

// 静态成员初始化
const char* HttpConn::srcDir;         // 资源目录路径（由外部设置）
std::atomic<int> HttpConn::userCount; // 原子连接计数器初始化为0
bool HttpConn::isET;                  // ET模式标记（默认false，需外部设置）

// 构造函数
HttpConn::HttpConn() { 
    fd_ = -1;                // 初始无效文件描述符
    addr_ = { 0 };           // 清空地址结构
    isClose_ = true;         // 初始为关闭状态
}

// 析构函数
HttpConn::~HttpConn() { 
    Close();                 // 确保连接关闭
};

/* 初始化新连接
 * @param fd 客户端socket描述符
 * @param addr 客户端地址结构
 */
void HttpConn::init(int fd, const sockaddr_in& addr) {
    assert(fd > 0);        // 确保有效fd
    userCount++;
    addr_ = addr;
    fd_ = fd;
    writeBuff_.RetrieveAll();  // 清空写缓冲区
    readBuff_.RetrieveAll();   // 清空读缓冲区
    isClose_ = false;          // 标记为打开状态
    // 记录日志：客户端IP、端口和当前连接数
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
}

void HttpConn::Close() {
    response_.UnmapFile();     // 释放内存映射文件（如果有）
    if(isClose_ == false){
        isClose_ = true;      // 标记为已关闭
        userCount--;          // 连接数-1
        close(fd_);           // 关闭socket
        // 记录客户端退出日志
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
    }
}

// 获取文件描述符
int HttpConn::GetFd() const {
    return fd_;
};
// 获取客户端地址结构
struct sockaddr_in HttpConn::GetAddr() const {
    return addr_;
}
// 获取客户端IP字符串（点分十进制）
const char* HttpConn::GetIP() const {
    return inet_ntoa(addr_.sin_addr);  //还有记得大端转小端
}
// 获取客户端端口号（网络字节序）
int HttpConn::GetPort() const {
    return addr_.sin_port;
}

/* 读取数据到读缓冲区
 * @param saveErrno 用于保存错误码的输出参数
 * @return 读取的字节数（-1表示错误）
 */
ssize_t HttpConn::read(int* saveErrno) {
    ssize_t len = -1;
    do{
        // 使用Buffer的ReadFd读取数据到读缓冲区
        len = readBuff_.ReadFd(fd_, saveErrno);
        if (len <= 0) {  // 出错或读完了,关闭
            break;
        }
    }while(isET); // ET模式需一次性读完所有数据 //这里要理解一下为什么ET模式要一次性读完
    return len;
}

/* 写数据到socket（使用writev聚合写）
 * @param saveErrno 保存错误码的输出参数
 * @return 写入的字节数（-1表示错误）
 */
ssize_t HttpConn::write(int* saveErrno) {
    ssize_t len = -1;
    do{
        len = writev(fd_,iov_,iovCnt_); // writev将iov数组内容写入socket
        if(len <= 0) {
            *saveErrno = errno;
            break;
        }
        // 判断数据是否全部写完
        if(iov_[0].iov_len + iov_[1].iov_len  == 0) { 
            break; /* 传输结束 */
        }else if(static_cast<size_t>(len) > iov_[0].iov_len){  // 处理部分写入情况（先写iov[0]响应头，再写iov[1]文件内容）
            // 这里详细解释一下,很容易搞不懂(先看下面那个else)
            // OK 这里仿照 下面那个else就好理解了
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len); // iov_[0]已写完，调整iov_[1]的指针和长度
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            // 但是着一块又难理解了
            // 首先咱们要知道一点 writeBuff_ 里面存储的是响应头部分,刚开始被赋予了iov_[0].iov_base,因此当iov_[0]写完了那么writeBuff_就没有用了,就清空一下就好了
            // 我们主要理解一下为什么到这个位置了iov_[0].iov_len还不为0
            // 举个例子好理解,比如原始的iov_[0].iov_len = 100 原始的iov_[1].iov_len = 1000 那么第一次写的时候len为200,那么是不是就理解了,这个时候不会进入下面那个else,只会进入这个,因此我们需要处理iov_[0]
            if(iov_[0].iov_len) {
                writeBuff_.RetrieveAll(); // 清空写缓冲区
                iov_[0].iov_len = 0;      // iov_[0]长度置零
            }
        }else{ // iov_[0]未写完，调整其指针和长度
            // 里详细解释一下,很容易搞不懂
            // 写的时候总得知道写的其实地址和写的总长度把
            // 刚开始第一个缓冲区的起始地址是 iov_[0].iov_base 和 长度是 iov_[0].iov_len  这个没的说
            // 到这里表示前面的 len = writev(fd_,iov_,iovCnt_);  正常执行了,并且已经写进入len这么多了,而且len要小于iov_[0].iov_len表示iov_[0]还没写完
            // 这个时候就需要调整一下后面写的其实地址和写的总长度了
            // 那么后面就好理解了 起始地址不就是 iov_base往后移动 len 个位置马
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len; 
            // 剩下的长度不就是iov_len减去len马
            iov_[0].iov_len -= len; 
            writeBuff_.Retrieve(len); // 更新写缓冲区读指针
        }
    }while(isET || ToWriteBytes() > 10240); // ET模式或数据量大时循环写 这里同样理解一下为什么T模式要一次性写完
    return len;
}

/* 处理HTTP请求生成响应
 * @return 处理是否成功
 */
bool HttpConn::process() {
    request_.Init();    // 初始化请求对象
    if(readBuff_.ReadableBytes() <= 0) {
        return false;   // 无数据可处理
    }else if(request_.parse(readBuff_)){     // 解析请求（成功则生成200响应，失败400）
        LOG_DEBUG("%s", request_.path().c_str());
        response_.Init(srcDir, request_.path(), request_.IsKeepAlive(), 200);
    }else{
        response_.Init(srcDir, request_.path(), false, 400);
    }
    // 生成响应到写缓冲区
    response_.MakeResponse(writeBuff_); // 生成响应报文放入writeBuff_中
    // 设置iov[0]指向响应头数据
    iov_[0].iov_base = const_cast<char*>(writeBuff_.Peek());
    iov_[0].iov_len = writeBuff_.ReadableBytes();
    iovCnt_ = 1;  // 默认只有响应头

    // 如果存在内存映射文件（如请求的文件），设置iov[1]
    if(response_.FileLen() > 0  && response_.File()) {
        iov_[1].iov_base = response_.File();
        iov_[1].iov_len = response_.FileLen();
        iovCnt_ = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", response_.FileLen() , iovCnt_, ToWriteBytes());
    return true;
}