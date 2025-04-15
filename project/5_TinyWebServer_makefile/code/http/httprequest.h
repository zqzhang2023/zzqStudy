#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>          // 正则表达式
#include <errno.h>        // 错误号处理
#include <mysql/mysql.h>  //mysql

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"

// #include "buffer.h"
// #include "log.h"
// #include "sqlconnpool.h"

class HttpRequest {
public:
    // 定义HTTP解析状态机的状态
    enum PARSE_STATE {
        REQUEST_LINE,  // 正在解析请求行（如 GET / HTTP/1.1）
        HEADERS,       // 正在解析请求头部
        BODY,          // 正在解析请求体
        FINISH,        // 解析完成
    };

    HttpRequest() { Init(); }  // 构造函数初始化解析状态
    ~HttpRequest() = default;  // 默认析构函数

    // 成员函数声明
    void Init();               // 初始化/重置解析器状态
    bool parse(Buffer& buff);  // 主解析函数，传入数据缓冲区
    // 获取解析结果的访问方法
    std::string path() const;      // 获取请求路径（常量）
    std::string& path();           // 获取请求路径（可修改）
    std::string method() const;    // 获取请求方法
    std::string version() const;   // 获取HTTP版本
    std::string GetPost(const std::string& key) const;   // 获取POST表单值
    std::string GetPost(const char* key) const;           // 重载版本

    bool IsKeepAlive() const;    // 判断是否保持连接

private:

    bool ParseRequestLine_(const std::string& line);    // 处理请求行
    void ParseHeader_(const std::string& line);         // 处理请求头
    void ParseBody_(const std::string& line);           // 处理请求体

    void ParsePath_();                                  // 处理请求路径
    void ParsePost_();                                  // 处理Post事件
    void ParseFromUrlencoded_();                        // 从url种解析编码

    // 用户验证（静态方法，与具体请求实例无关）
    static bool UserVerify(const std::string& name, const std::string& pwd, bool isLogin);  // 用户验证

    // 成员变量
    PARSE_STATE state_;                        // 当前解析状态
    std::string method_,path_,version_,body_;  // 请求方法（GET/POST等） 请求路径  HTTP版本（如HTTP/1.1） 请求体内容
    std::unordered_map<std::string, std::string> header_;   // 请求头键值对
    std::unordered_map<std::string, std::string> post_;     // POST表单键值对
    // 静态常量：预定义的HTML页面和标签
    static const std::unordered_set<std::string> DEFAULT_HTML;   // 默认支持的页面
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG; // 页面类型标记
    static int ConverHex(char ch);  // 十六进制字符转换辅助函数
};

#endif