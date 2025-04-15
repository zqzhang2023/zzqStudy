#include "httpresponse.h"

using namespace std;

// HTTP/1.1 200 OK\r\n
// Date: Fri, 22 May 2009 06:07:21 GMT\r\n
// Content-Type: text/html; charset=UTF-8\r\n
// \r\n
// <html>
//       <head></head>
//       <body>
//             <!--body goes here-->
//       </body>
// </html>


// 文件后缀到MIME类型的映射
// 告诉浏览器如何处理文件
// 例如：text/html 表示 HTML 文档，浏览器会渲染它；application/pdf 表示 PDF 文件，浏览器可能会调用 PDF 阅读器。
const unordered_map<string, string> HttpResponse::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

// 状态码到状态描述的映射
const unordered_map<int, string> HttpResponse::CODE_STATUS = {
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

// 状态码到错误页面路径的映射
const unordered_map<int, string> HttpResponse::CODE_PATH = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};

// 构造函数初始化成员变量
HttpResponse::HttpResponse() {
    code_ = -1;              // 初始无效状态码
    path_ = srcDir_ = "";    // 清空路径
    isKeepAlive_ = false;    // 默认关闭长连接
    mmFile_ = nullptr;       // 初始无内存映射
    mmFileStat_ = { 0 };     // 清空文件状态
};

// 析构函数确保解除内存映射
HttpResponse::~HttpResponse() {
    UnmapFile();
}


// 初始化函数
void HttpResponse::Init(const string& srcDir, string& path, bool isKeepAlive, int code){
    assert(srcDir != "");             // 确保资源目录有效
    if(mmFile_){
        UnmapFile();                  // 首先解除之前的内存映射
    }
    // 设置成员变量
    code_ = code;
    isKeepAlive_ = isKeepAlive;
    path_ = path;
    srcDir_ = srcDir;
    mmFile_ = nullptr; 
    mmFileStat_ = { 0 };
}

// 构建HTTP响应主函数
void HttpResponse::MakeResponse(Buffer& buff){
    /* 判断请求的资源文件 */
    // 检查文件是否存在且不是目录
    if(stat((srcDir_ + path_).data(), &mmFileStat_) < 0 || S_ISDIR(mmFileStat_.st_mode)) {
        code_ = 404;   // 不存在或为目录
    }else if(!(mmFileStat_.st_mode & S_IROTH)){
        code_ = 403;   // 无读取权限
    }else if(code_ == -1){ 
        code_ = 200;   // 未指定状态码时默认成功
    }
    ErrorHtml_();           // 处理错误页面路径 这里相当于先判断一下是否存在错误(400,403,404)，如果存在的话就把文件换成响应的错误的页面
    AddStateLine_(buff);    // 构建状态行
    AddHeader_(buff);       // 构建头部
    AddContent_(buff);      // 构建内容
}

// 获取内存映射文件指针\r\n
char* HttpResponse::File() {
    return mmFile_;
}

// 获取文件长度
size_t HttpResponse::FileLen() const {
    return mmFileStat_.st_size;  // 从文件状态获取大小
}

// 处理错误页面路径
void HttpResponse::ErrorHtml_() {
    // 如果当前状态码有对应错误页面
    if(CODE_PATH.count(code_) == 1) {
        path_ = CODE_PATH.find(code_)->second;         // 更新路径为错误页面
        stat((srcDir_ + path_).data(), &mmFileStat_);  // 重新获取文件状态
    }
}

// 添加状态行
void HttpResponse::AddStateLine_(Buffer& buff) {
    string status;
    if(CODE_STATUS.count(code_) == 1) {  // 已知状态码
        status = CODE_STATUS.find(code_)->second;
    }else{
        code_ = 400;    // 未知状态码默认400
        status = CODE_STATUS.find(400)->second;
    }
    // 格式：HTTP/1.1 200 OK\r\n
    buff.Append("HTTP/1.1 " + to_string(code_) + " " + status + "\r\n");
}

// 添加响应头
void HttpResponse::AddHeader_(Buffer& buff) {
    // 连接状态
    buff.Append("Connection: ");
    if(isKeepAlive_){
        buff.Append("keep-alive\r\n");
        buff.Append("keep-alive: max=6, timeout=120\r\n");
    }else{
        buff.Append("close\r\n");
    }
    // 内容类型
    buff.Append("Content-type: " + GetFileType_() + "\r\n");
}

// 添加响应内容
void HttpResponse::AddContent_(Buffer& buff) {
    // 打开文件
    int srcFd = open((srcDir_ + path_).data(), O_RDONLY);
    if(srcFd < 0) { 
        ErrorContent(buff, "File NotFound!");  // 文件打开失败
        return; 
    }
    //将文件映射到内存提高文件的访问速度  MAP_PRIVATE 建立一个写入时拷贝的私有映射
    LOG_DEBUG("file path %s", (srcDir_ + path_).data());
        // mmap 是一个系统调用，用于将文件或设备映射到进程的地址空间，从而可以通过指针操作来访问文件内容。
    // 表示映射的地址由系统选择。 mmFileStat_.st_size：表示映射的长度，即文件的大小。mmFileStat_ 是一个 struct stat 类型的变量，st_size 是文件的大小。
    // PROT_READ表示映射区域的保护属性，只允许读取。  MAP_PRIVATE表示映射是私有的，对映射区域的修改不会写回文件。 文件描述符，表示要映射的文件。0 表示从文件的偏移量为 0 的位置开始映射。
    // 返回一个指向映射区域的指针。如果映射失败，返回 (void*)-1。 这里将返回值强制转换为 int* 类型，并赋值给 mmRet。
    int* mmRet = (int*)mmap(0, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if(*mmRet == -1) {
        ErrorContent(buff, "File NotFound!");   // 映射失败
        return; 
    }
    mmFile_ = (char*)mmRet;  // 保存映射指针
    close(srcFd);
    // 添加内容长度头
    buff.Append("Content-length: " + to_string(mmFileStat_.st_size) + "\r\n\r\n"); //注意看上面的HTTP响应报文格式，就知道这里为啥会有两个"\r\n\r\n"了
}

// 解除内存映射
void HttpResponse::UnmapFile() {
    if(mmFile_) {
        munmap(mmFile_, mmFileStat_.st_size); // 释放映射内存
        mmFile_ = nullptr;
    }
}

// 判断文件类型 
string HttpResponse::GetFileType_() {
    // 查找最后一个.的位置 得找到后缀，然后查表
    string::size_type idx = path_.find_last_of('.');
    if(idx == string::npos) {   // 最大值 find函数在找不到指定值得情况下会返回string::npos 这个说明没有 .  也就是说没有后缀
        return "text/plain";    // 默认纯文本
    }
    string suffix = path_.substr(idx);   // 提取后缀
    if(SUFFIX_TYPE.count(suffix)==1){    // 查找已知类型
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";    // 默认纯文本
}

// 生成错误内容HTML
void HttpResponse::ErrorContent(Buffer& buff, string message) 
{
    string body;
    string status;
    // 构建HTML结构
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if(CODE_STATUS.count(code_) == 1) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        status = "Bad Request";
    }
    // 填充错误信息
    body += to_string(code_) + " : " + status  + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";
    // 添加内容长度和内容本身
    buff.Append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}