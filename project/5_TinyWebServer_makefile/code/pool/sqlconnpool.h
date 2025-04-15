#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "../log/log.h"

/* 数据库连接池类（单例模式） */
class SqlConnPool{
private:

    int MAX_CONN_; //最大连接数
    std::queue<MYSQL* > connQue_; //连接队列
    std::mutex mtx_;  // 互斥锁
    sem_t semId_;     // 信号量（控制并发访问） PV操作

    // 私有化构造函数
    SqlConnPool() = default;
    // 析构时自动关闭
    ~SqlConnPool() { ClosePool(); }
public:
    // 获取单例实例（C++11线程安全实现）
    static SqlConnPool* Instance();
    // 获取一个数据库连接
    MYSQL* GetConn();
    // 释放连接回池
    void FreeConn(MYSQL* conn);
    // 获取空闲连接数量
    int GetFreeConnCount();
    /* 初始化连接池
     * @param host:     数据库主机地址
     * @param port:     端口号
     * @param user:     用户名
     * @param pwd:      密码
     * @param dbName:   数据库名
     * @param connSize: 连接池大小
     */
    void Init(const char* host, uint16_t port,
        const char* user,const char* pwd, 
        const char* dbName, int connSize);

    // 关闭连接池
    void ClosePool();
};


/* 资源在对象构造初始化 资源在对象析构时释放  RAII数据库连接管理类*/
class SqlConnRAII{
private:
    MYSQL *sql_;               // 持有的数据库连接
    SqlConnPool* connpool_;    // 关联的连接池
public:
    /* 构造函数获取连接
     * @param sql:     输出参数，用于返回获得的连接
     * @param connpool: 连接池实例
     */
    SqlConnRAII(MYSQL** sql, SqlConnPool *connpool){
        assert(connpool);
        *sql = connpool->GetConn();   // 从池中获取连接
        sql_ = *sql;                  // 保存连接指针
        connpool_ = connpool;         // 保存连接池引用
    }
    // 析构时自动释放连接
    ~SqlConnRAII() {
        if(sql_) { connpool_->FreeConn(sql_); }
    }
};

#endif // SQLCONNPOOL_H