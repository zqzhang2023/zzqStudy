#include "httprequest.h"
using namespace std;

// // 定义HTTP解析状态机的状态
// enum PARSE_STATE {
//     REQUEST_LINE,  // 正在解析请求行（如 GET / HTTP/1.1）
//     HEADERS,       // 正在解析请求头部
//     BODY,          // 正在解析请求体
//     FINISH,        // 解析完成
// };

// 预定义的默认支持HTML页面（自动添加.html后缀）
const unordered_set<string> HttpRequest::DEFAULT_HTML {
    "/index", "/register", "/login", "/welcome", "/video", "/picture",
};

// 页面类型映射（登录页为1，注册页为0）
const unordered_map<string, int> HttpRequest::DEFAULT_HTML_TAG {
    {"/login.html", 1}, {"/register.html", 0}
};

// 初始化HTTP请求解析器状态 
void HttpRequest::Init() {
    state_ = REQUEST_LINE;                    // 初始状态为解析请求行
    method_ = path_ = version_= body_ = "";   // 清空请求方法 请求路径 HTTP版本  请求体
    header_.clear();    // 清空请求头
    post_.clear();      // 清空POST数据
}

// 主解析函数（核心状态机）
bool HttpRequest::parse(Buffer& buff) {
    const char END[] = "\r\n";      // HTTP换行符定义  HTTP每一行都是以 "\r\n" 结尾的 (请求体不是哈)
    if(buff.ReadableBytes() == 0){  // 没有可读的字节
        return false;
    }
    // 读取数据开始 当缓冲区有数据且未完成解析时循环处理
    while(buff.ReadableBytes() && state_!= FINISH){
        // 从buff中的读指针开始到读指针结束，这块区域是未读取得数据并去处"\r\n"，返回有效数据得行末指针
        const char* lineend = search(buff.Peek(), buff.BeginWriteConst(), END, END+2);  //查找子串 返回第一个子串的首地址
        string line(buff.Peek(),lineend); // 提取当前行内容（不包含换行符）这样就是一个完整的报文行了
        switch (state_){ // 根据当前状态处理不同部分
            case REQUEST_LINE:{
                bool res = ParseRequestLine_(line); //解析请求行，返返回解析结果
                if(!res){
                    return false;
                }
                ParsePath_();   // 处理请求路径（添加.html后缀等）
                break;
            }
            case HEADERS:{
                ParseHeader_(line);  // 解析头部字段
                if(buff.ReadableBytes() <= 2) {  // 说明是get请求，后面为\r\n 如果剩余数据<=2字节（只剩\r\n），说明没有请求体，直接完成
                    state_ = FINISH;   // 提前结束
                }
                break;
            }
            case BODY:{
                ParseBody_(line);  // 处理请求体
                break;
            }
            default:{
                break;
            }
        }

        // 处理缓冲区：移动读指针到下一行开始
        // 处理缓冲区：移动读指针到下一行开始
        if(lineend == buff.BeginWrite()) {  // 读完了
            buff.RetrieveAll();
            break;
        }
        buff.RetrieveUntil(lineend + 2);        // 跳过已处理的\r\n
    }
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str()); //这就是之前提的带参的
    return true;
}

// 解析请求行（格式：方法 路径 HTTP/版本） // 注意一正则操作哈
bool HttpRequest::ParseRequestLine_(const string& line) {
    regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");  //  // 正则表达式匹配三部分
    smatch Match;   // 用来匹配patten得到结果
    // 在匹配规则中，以括号()的方式来划分组别 一共三个括号 [0]表示整体
    if(regex_match(line, Match, patten)) {  // 匹配指定字符串整体是否符合
        method_ = Match[1];  // 第1组：方法（GET/POST）
        path_ = Match[2];    // 第2组：请求路径
        version_ = Match[3]; // 第3组：HTTP版本
        state_ = HEADERS;    // 状态转移到解析头部
        return true;
    }
    LOG_ERROR("RequestLine Error");
    return false;
}

// 解析路径，统一一下path名称,方便后面解析资源（添加默认页面或后缀）
void HttpRequest::ParsePath_() {
    if(path_ == "/") { //说明刚来请求，还没请求东西，先来给个首页
        path_ = "/index.html";
    } else {
        // 检查是否是预定义的页面，自动添加.html后缀
        if(DEFAULT_HTML.find(path_) != DEFAULT_HTML.end()) {
            path_ += ".html";
        }
    }
}

// 解析单个请求头字段（格式：Key: Value）
void HttpRequest::ParseHeader_(const string& line) {
    regex patten("^([^:]*): ?(.*)$"); // 匹配键值对
    smatch Match;
    if(regex_match(line, Match, patten)) {
        header_[Match[1]] = Match[2];   // 存储到header_哈希表
    } else {    // 匹配失败说明首部行匹配完了，状态变化
        state_ = BODY;  // 转移到解析请求体状态
    }
}

// 处理请求体（目前直接存储，后续处理由ParsePost_完成）
void HttpRequest::ParseBody_(const string& line) {
    body_ = line;       // 暂存请求体内容
    ParsePost_();       // 解析POST数据
    state_ = FINISH;    // 状态转换为下一个状态
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

// 16进制转化为10进制
int HttpRequest::ConverHex(char ch) {
    if (ch >= 'A' && ch <= 'F') 
        return ch - 'A' + 10;     // A(10) ~ F(15)
    if (ch >= 'a' && ch <= 'f') 
        return ch - 'a' + 10;     // a(10) ~ f(15)
    if (ch >= '0' && ch <= '9') 
        return ch - '0';          // 0(0) ~9(9)
    return 0;  // 非法字符可返回0或抛出异常
}

// 处理POST请求（表单数据解析和用户验证）
void HttpRequest::ParsePost_() {
    // 检查是否为表单提交
    if(method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded") {
        // 解析URL编码的表单数据（处理%编码和键值对） 比如：name=John%20Doe&age=25%26under20
        ParseFromUrlencoded_();

        // 检查当前路径是否需要验证（登录/注册）
        if(DEFAULT_HTML_TAG.count(path_)){ // 如果是登录/注册的path
            int tag = DEFAULT_HTML_TAG.find(path_)->second;   // 获取页面类型标记
            LOG_DEBUG("Tag:%d", tag);
            if(tag==0||tag==1){
                bool isLogin = (tag == 1);  // 为1则是登录，处理登陆（数据库select，处理注册的话数据库得insert 无论是登陆还是注册，数据库都得select哈，毕竟得确认用户名未被使用）
                if(UserVerify(post_["username"], post_["password"], isLogin)) {
                    path_ = "/welcome.html";  //匹配成功则path_改成welcome
                } 
                else {
                    path_ = "/error.html";    //匹配不成功则path_改成error
                }
            }
        }
    }
}

// 解析URL编码的表单数据（处理%编码和键值对）
// 比如：name=John%20Doe%26age=25
// 处理name=John%20Doe %20解码为空格，键值对变为name=John Doe。
// %26解码为&，键值对变为name=John Doe&age=25
void HttpRequest::ParseFromUrlencoded_() {
    if(body_.size() == 0) { return; }  //空则return

    string key, value;
    int num = 0;
    int n = body_.size();
    int i = 0, j = 0;

    for(; i < n; i++) {
        char ch = body_[i];
        switch (ch) {
        case '=': // 键值分隔符
            key = body_.substr(j, i - j);
            j = i + 1;
            break;
        case '+': // 空格替换
            body_[i] = ' ';
            break;
        case '%': // 处理%编码（如%20为空格）
            num = ConverHex(body_[i + 1]) * 16 + ConverHex(body_[i + 2]);
            body_[i + 2] = num % 10 + '0';
            body_[i + 1] = num / 10 + '0';
            i += 2; // 跳过两个十六进制字符
            break;
        case '&':  // 键值对分隔符
            value = body_.substr(j, i - j);
            j = i + 1;
            post_[key] = value; //存储一下
            LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
            break;
        default:
            break;
        }
    }
    assert(j <= i);
    // 处理最后一个键值对
    if(post_.count(key) == 0 && j < i) {
        value = body_.substr(j, i - j);
        post_[key] = value;
    }
}

// 用户验证（查询数据库）
bool HttpRequest::UserVerify(const string &name, const string &pwd, bool isLogin) {
    if(name == "" || pwd == "") { return false; }  //为空直接返回
    LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());

    // 从连接池获取数据库连接（RAII方式自动释放）
    MYSQL* sql;
    SqlConnRAII(&sql, SqlConnPool::Instance()); // SqlConnRAII Name(&sql, SqlConnPool::Instance()); 只不过这里不需要name来操作
    assert(sql);  //判断是拿到正常的sql

    bool flag = false;  //这个flag 前边表示是否需要执行注册(insert) 后面表示执行的结果 成功与否
    unsigned int j = 0;
    char order[256] = { 0 };
    MYSQL_FIELD *fields = nullptr;
    MYSQL_RES *res = nullptr;

    if(!isLogin){
        flag = true; //说明是注册操作 数据库得 insert 登陆的话 数据库只需要select  无论是登陆还是注册，数据库都得select哈，毕竟得确认用户名未被使用
    }

    // 查询用户及密码
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", name.c_str());
    LOG_DEBUG("%s", order);

    // 执行查询
    if(mysql_query(sql,order)){
        mysql_free_result(res);
        return false; 
    }

    res = mysql_store_result(sql);
    j = mysql_num_fields(res);
    fields = mysql_fetch_fields(res);

    // 处理查询结果
    while(MYSQL_ROW row = mysql_fetch_row(res)){
        LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
        string password(row[1]); // 数据库中的密码
        if(isLogin){  //登陆表单 比对一下密码
            if(pwd == password) { 
                flag = true; 
            }else{
                flag = false;
                LOG_INFO("pwd error!");
            }
        }else{ //注册表单 //既然能够查询到用户名和密码，那么说明用户名已经存在了
            flag = false;
            LOG_INFO("user used!");
        }
    }
    mysql_free_result(res);

    // 注册行为 且 用户名未被使用
    if(!isLogin && flag == true) {
        LOG_DEBUG("regirster!");
        bzero(order, 256);
        snprintf(order, 256,"INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(), pwd.c_str());
        LOG_DEBUG( "%s", order);
        if(mysql_query(sql, order)) { 
            LOG_DEBUG( "Insert error!");
            flag = false; 
        }
        flag = true;
    }

    LOG_DEBUG( "UserVerify success!!");
    // 这里要注意哈， SqlConnRAII 会自动释放，执行析构函数，也就是说自动释放sql资源到池子里面，注意回去看看这个是怎么操作的 
    return flag;
}

std::string HttpRequest::path() const{
    return path_;
}

std::string& HttpRequest::path(){
    return path_;
}
std::string HttpRequest::method() const {
    return method_;
}

std::string HttpRequest::version() const {
    return version_;
}

std::string HttpRequest::GetPost(const std::string& key) const {
    assert(key != "");
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}

std::string HttpRequest::GetPost(const char* key) const {
    assert(key != nullptr);
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}

// 判断是否保持连接
bool HttpRequest::IsKeepAlive() const {
    if(header_.count("Connection") == 1) {
        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}