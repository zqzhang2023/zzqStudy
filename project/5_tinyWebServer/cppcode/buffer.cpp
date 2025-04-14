#include "../include/buffer.h"

// 构造函数：初始化缓冲区大小和读写位置
Buffer::Buffer(int initBuffSize):buffer_(initBuffSize),readPos_(0),writePos_(0){
    assert(initBuffSize > 0 ); //确保initBuffSize的有效性
}

// 返回当前可写字节数 = 总容量 - 写位置
size_t Buffer::WritableBytes() const{
    return buffer_.size() - writePos_;
}

// 返回当前可读字节数 = 写位置 - 读位置
size_t Buffer::ReadableBytes() const{
    return writePos_ - readPos_;
}

// 返回预留空间（已读区域）大小 = 读位置 (已经读过的就没用了，等于读下标)
size_t Buffer::PrependableBytes() const{
    return readPos_;
}

// 返回可读数据起始地址（常量指针）
const char* Buffer::Peek() const{
    return &buffer_[readPos_];
}

// 确保至少有len字节可写空间，否则扩容
void Buffer::EnsureWriteable(size_t len){
    if(len > WritableBytes()){  //目前的空间不足
        MakeSpace_(len);        //整理(合并)一下空间，再不足就扩容了
    }
    assert(len < WritableBytes()); //打个断言
}

// 移动写下标，更新写位置（数据写入后调用)
void Buffer::HasWritten(size_t len){
    writePos_ += len; // 写指针后移len字节 注意：可能会有疑问，这里就不怕越界吗？其实一般情况下，要写入输入之前会调用EnsureWriteable，再写再移动指针，但是说实话还是程序控制比较好，现在的方法全靠程序员自觉（而且这还是个public方法）
}

// 读取len长度，移动读下标 消费数据：移动读指针
void Buffer::Retrieve(size_t len){
    readPos_ += len; //和HasWritten同理，如果跟着做完有时间的话，可以尝试一下重构一下代码
}

// 消费数据直到指定地址，这里没说要读哈，就是相当于把end之前的可读的区域都放弃掉
void Buffer::RetrieveUntil(const char* end){
    assert(Peek() <= end); //你看这这里就打断言了， &buffer_[readPos_] 要小于 end
    Retrieve(end - Peek()); // end指针 - 读指针 长度
}

// 取出所有数据，buffer归零，读写下标归零,在别的函数中会用到
void Buffer::RetrieveAll(){
    bzero(&buffer_[0], buffer_.size());  // 覆盖原本数据 清空数据 可选，因为readPos_ = writePos_ = 0; 其实就代表归0了，里面的内容就相当于全部舍弃了
    readPos_ = writePos_ = 0;
}

//  取出剩余可读的str 取出所有可读数据并转为字符串
std::string Buffer::RetrieveAllToStr(){
    std::string str(Peek(),ReadableBytes()); //string字符串构造，记得看了解一下字符串
    RetrieveAll();
    return str;
}

// 返回当前写位置常量指针
const char* Buffer::BeginWriteConst() const{
    return &buffer_[writePos_];
}

// 返回当前写位置指针
char* Buffer::BeginWrite(){
    return &buffer_[writePos_];
}

// 添加str到缓冲区
void Buffer::Append(const char* str, size_t len) {
    assert(str);
    EnsureWriteable(len);                       // 确保可写的长度
    std::copy(str, str + len, BeginWrite());    // 将str放到写下标开始的地方
    HasWritten(len);                            // 移动写下标
}

// 添加字符串到缓冲区
void Buffer::Append(const std::string& str){
    Append(str.c_str(),str.size());
}

// 任意类型，强制转换
void Buffer::Append(const void* data, size_t len) {
    Append(static_cast<const char*>(data), len);
}

// 添加另一个缓冲区的可读数据到当前缓冲区(不是合并俩缓冲区哈，就相当与把buff之中的可读数据写在现在这个缓冲区里面哈，别搞错了)
void Buffer::Append(const Buffer& buff){
    Append(buff.Peek(), buff.ReadableBytes());
}

// 获取缓冲区起始地址（内部使用）
char* Buffer::BeginPtr_() {
    return &buffer_[0];
}

// 获取缓冲区起始地址（常量版本）
const char* Buffer::BeginPtr_() const{
    return &buffer_[0];
}

// 扩展空间
void Buffer::MakeSpace_(size_t len) {
    if(WritableBytes() + PrependableBytes() < len){  // C + A 区域 都无法满足 len大小的数据了，这时候就要扩容了
        buffer_.resize(writePos_ + len + 1); //  扩容至所需大小+1（保险）
    }else{  //合并空间
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_()); //将缓冲区中从readPos_到writePos_之间的数据复制到缓冲区的起始位置
        readPos_ = 0;          // 读指针归零
        writePos_ = readable;  // 写指针指向原数据末尾
        assert(readable == ReadableBytes()); // 确保数据完整性
    }
}

// 两个最为重要的函数，放在最后来写
// 将buffer中可读的区域写入fd中
ssize_t Buffer::WriteFd(int fd,int* Errno){
    ssize_t len = write(fd,Peek(),ReadableBytes()); //将读区域内部的所有的东西都写进fd之中，一般情况下都不会出错，也就是 len 会= ReadableBytes()，但是也有可能会出错
    if(len < 0){
        *Errno = errno;
        return len;
    }
    Retrieve(len);
    return len;
}

// 将fd的内容读到缓冲区，即writable的位置
// 这里要注意哈，看咱们前面的函数，一般情况下，咱们会调用咱们的Append函数来向buffer里面写东西
// 但是碰到一个问题，如何判断咱们要写入的长度，这取决于fd要传给咱们多少东西，咱们现在也不知道，要传给咱多少
// 因此这里要使用 分散读写 通过定义一个 struct iovec 数组接收, struct iovec iov[2]; ssize_t len = readv(fd, iov, 2); 会优先向iov[0]之中写,如果不够写的话,会再向iov[1]写
// struct iovec {
//     void  *iov_base;  // 指向缓冲区的指针
//     size_t iov_len;   // 缓冲区的长度
// };
ssize_t Buffer::ReadFd(int fd,int* Errno){
    char buff[65535];   // 栈区
    struct iovec iov[2];
    size_t writeable = WritableBytes(); // 获取现在缓冲区内部还有多少可写的空间
    // 分散读,保证数据能够读完
    iov[0].iov_base = BeginWrite();
    iov[0].iov_len = writeable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff); //65535

    ssize_t len = readv(fd, iov, 2); //开始读
    if(len < 0){ // 出现错误了
        *Errno = errno;
    }else if(static_cast<size_t>(len) <= writeable){ //说明咱们基础的缓存空间是足够存储的,没有使用char buff[65535]; 
        writePos_ += len;   // 直接移动写下标 //你看你看,我就说HasWritten函数有问题,这里估计作者自己都忘记了有这么一个函数
    }else{ // 说明咱们的空间是不够的(同时说明了,咱们的buffer_空间内部已经填满了)
        writePos_ = buffer_.size(); // 写区写满了,下标移到最后
        Append(buff, static_cast<size_t>(len - writeable)); // 剩余的长度
    }
    return len;
}