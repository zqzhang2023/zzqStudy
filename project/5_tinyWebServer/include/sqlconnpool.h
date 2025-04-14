#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "log.h"

//数据库连接池类
//1.数据库连接池。注意：服务器在运行的时候肯定不只一个用户连接，每个用户连接的时候都需要进行数据库操作，在连接的时候要对数据库连接进行初始化，那样就非常的耗时，因此引入了 数据库连接池
// --先进行数据库连接，把连接好的标识放入一个池子里面（这里是queue），然后用户来的时候只需要把这些连接好的标识非配给他就好了
//2.单例模式。 整个系统就只需要一个池子，不需要多个池子，也就是说不需要多个实例，如果想要更多的池子的话，把的第一个池子扩大一些不久好了吗
//3.RAII，这个就是说，由系统来管理资源的申请和释放。和智能指针就是用这个来思路来实现的，咱们不需要手动释放资源了，系统会帮忙管理
//4.看这份代码之前先了解一下基本的数据库操作

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
    /* 构造函数获取SqlConnRAIInnpool: 连接池实例
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