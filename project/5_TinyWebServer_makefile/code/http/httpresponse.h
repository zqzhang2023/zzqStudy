#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <unordered_map>
#include <fcntl.h>       // open
#include <unistd.h>      // close
#include <sys/stat.h>    // stat
#include <sys/mman.h>    // mmap, munmap

#include "../buffer/buffer.h"
#include "../log/log.h"

class HttpResponse {
private:
    // 成员变量
    int code_;                           // HTTP状态码
    bool isKeepAlive_;                   // 是否保持长连接
    std::string path_;                   // 也就是文件名
    std::string srcDir_;                 // 资源的根目录（绝对）

    char* mmFile_;                       // 内存映射的文件指针
    struct stat mmFileStat_;             // 文件状态信息

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;  // 后缀类型集
    static const std::unordered_map<int, std::string> CODE_STATUS;          // 编码状态集
    static const std::unordered_map<int, std::string> CODE_PATH;            // 编码路径集

    void AddStateLine_(Buffer &buff);    // 添加状态行到缓冲区
    void AddHeader_(Buffer &buff);       // 添加响应头到缓冲区
    void AddContent_(Buffer &buff);      // 添加响应内容到缓冲区
    void ErrorHtml_();                   // 处理错误页面路径
    std::string GetFileType_();          // 获取文件类型

public:
    HttpResponse();  // 构造函数
    ~HttpResponse(); // 析构函数

    // 初始化HTTP响应对象
    // 参数: srcDir-资源根目录，path-请求路径，isKeepAlive-是否保持连接，code-初始状态码
    void Init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1);
    // 构建完整的HTTP响应到缓冲区
    void MakeResponse(Buffer& buff);
    // 获取内存映射的文件指针
    char* File();
    // 解除内存映射
    void UnmapFile();
    // 获取文件长度
    size_t FileLen() const;
    // 生成错误内容到缓冲区
    void ErrorContent(Buffer& buff, std::string message);
    // 获取当前状态码
    int Code() const { return code_; }
};


#endif //HTTP_RESPONSE_H