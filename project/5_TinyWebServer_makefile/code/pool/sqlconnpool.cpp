#include "sqlconnpool.h"

// 获取单例实例（线程安全）
SqlConnPool* SqlConnPool::Instance() {
    static SqlConnPool pool; // C++11保证静态变量初始化线程安全
    return &pool;
}

/* 初始化数据库连接池
 * @param host     : 数据库主机名/IP
 * @param port     : 端口号（无符号16位）
 * @param user     : 用户名
 * @param pwd      : 密码
 * @param dbName   : 数据库名称
 * @param connSize : 连接池容量
 */
void SqlConnPool::Init(const char* host, uint16_t port,
    const char* user,const char* pwd, 
    const char* dbName, int connSize = 10) {

    assert(connSize > 0); // connSize 连接池容量 得大于 0 
    for(int i=0;i<connSize;i++){   // 创建连接
        MYSQL* conn = nullptr;
        conn = mysql_init(conn);   // 初始化MYSQL结构体
        if(!conn){
            LOG_ERROR("MySql init error!");     // 记录错误日志
            assert(conn);                       // 调试模式终止
        }
        // 建立实际连接
        conn = mysql_real_connect(conn, host, user, pwd, dbName, port, nullptr, 0);
        if(!conn){
            LOG_ERROR("MySql Connect error!");   // 连接失败记录
        }
        connQue_.emplace(conn);   // 将连接加入队列
    }
    MAX_CONN_ = connSize;
    sem_init(&semId_, 0, MAX_CONN_);  // 初始化信号量（进程间不共享，初始值为MAX_CONN_）
}

/* 获取数据库连接
 * @return: 可用MYSQL连接指针，无可用时返回nullptr
 */
MYSQL* SqlConnPool::GetConn() {
    MYSQL* conn = nullptr;
    if(connQue_.empty()){
        LOG_WARN("SqlConnPool busy!");    // 记录警告日志
        return nullptr;                   // 无可用时返回nullptr 
    }
    sem_wait(&semId_);                    // P操作（信号量-1），阻塞直到>0
    lock_guard<mutex> locker(mtx_);       // 加锁
    conn = connQue_.front();              // 获取队首连接
    connQue_.pop();                       // 移除队列元素
    return conn;                          // 可用MYSQL连接指针
}

/* 释放连接回池（实际上没有释放，只是放到池子里面去了）
 * @param conn: 要释放的数据库连接
 */
void SqlConnPool::FreeConn(MYSQL* conn) {
    assert(conn);                       // 确保连接有效
    lock_guard<mutex> locker(mtx_);
    connQue_.push(conn);                // 将连接放回队列
    sem_post(&semId_);                  // V操作（信号量+1）
}

// 关闭连接池（释放所有连接）
void SqlConnPool::ClosePool() {
    lock_guard<mutex> locker(mtx_);
    while(!connQue_.empty()) {    // 关闭数据库连接
        auto conn = connQue_.front(); 
        connQue_.pop();
        mysql_close(conn);
    }
    mysql_library_end();  // 清理MySQL库资源
}

// 获取当前空闲连接数
int SqlConnPool::GetFreeConnCount() {
    lock_guard<mutex> locker(mtx_);
    return connQue_.size();
}