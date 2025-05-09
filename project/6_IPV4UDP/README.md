# 基于IPV4的流媒体广播

这个项目和webserver挺像的，但是这个是基于UDP的，实现了一个组播。有服务端和客户端两个。

服务端：主要是不停的向外广播自己的节目单和节目内容（没错，就算没有客户端开启，也会一直广播）

客户端：主要是根据自己的需求订阅服务端的节目单，然后接收相应的节目数据，传递给ffmpeg，让ffmpeg来解析一下内容，然后播放出来
（是不是感觉挺高大上的，其实不是，只是简单的接收所有的数据，然后判断这个数据里面的ID是不是自己选择的就好，是自己选择的就保留，不是就丢弃）

# 运行起来

原项目git：https://github.com/litbubo/Streaming_media_broadcasting_system_based_on_IPv4

环境: Ubuntu 20.04 ffmpeg

还需要下载一个ffmepg包，这个是啥包我忘记了，但是在运行client的时候会发现只会接收一个UDP包的数据

这是因为缺少了这个包。接收数据之后会馈入ffmpeg里面，因为缺少了这个包，所以才会只接收一个数据之后就断掉

终端上会有提示，到时候直接ubuntu命令下载即可

```cpp
sudo cp -r ./medialib/ /var/
// 终端-1 server
cd server
make
./server

// 终端-2 client
cd client
./start.sh

(这里不是简单的接收数据，ffmpeg是可以正常播放出来音乐的，放不出来可能是虚拟机的问题，我这边是可以正常放出来的)
```

# 结构

```
6_IPV4UDP/
├── client
│   ├── client.c
│   ├── client.h
│   ├── Makefile
│   └── start.sh
├── include
│   └── protocol.h
├── medialib
│   ├── ch1
│   │   ├── desc.txt
│   │   ├── 世界这么大还是遇见你.mp3
│   │   └── 起风了.mp3
│   ├── ch2
│   │   ├── desc.txt
│   │   ├── 带你去旅行.mp3
│   │   └── 再见只是陌生人.mp3
│   ├── ch3
│   │   ├── desc.txt
│   │   ├── Summertime Sadness.mp3
│   │   └── 学猫叫.mp3
│   └── ch4
├── server
│   ├── channel.c
│   ├── channel.h       //每个频道的广播
│   ├── list.c
│   ├── list.h          //节目单广播
│   ├── Makefile
│   ├── medialib.c
│   ├── medialib.h      //medialib转化为广播频道
│   ├── server.c
│   ├── server_conf.h   //服务端的一些基础的配置
│   ├── threadpool.c
│   ├── threadpool.h    //线程池
│   ├── tokenbucket.c
│   └── tokenbucket.h   //令牌桶，负责流量控制
├── README.md
├── LICENSE
└── Streaming_media_broadcasting_system_based_on_IPv4.code-workspace
```
# 复现

## 协议

protocol.h 文件，这里面定义了很多的基本的配置，比如：多播组的地址，默认的端口号，还有节目单，频道的格式等。

代码:这里面不涉及啥复杂的操作，看一看即可

```cpp
#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include<stdint.h>
#include<stdio.h>

typedef uint8_t chnid_t;    //定义频道ID类型为8位无符号整数，范围0~255，节省空间

#define DEFAULT_MGROUP     "224.2.2.2" // 多播组
#define DEFAULT_RECVPORT   "1989"      // 默认端口
#define CHANNR             100         // 最大频道数量

#define LISTCHNID          0           // 约定 0 号为节目单频道
#define MINCHNID           1           // 最小广播频道号
#define MAXCHNID           (CHANNR - MINCHNID)  // 最大广播频道号 100 - 1

#define MSG_CHANNEL_MAX    (65536U - 20U - 8U)  // 最大频道数据包  20U IP头   8U UDP头
#define MAX_CHANNEL_DATA   (MSG_CHANNEL_MAX - sizeof(chnid_t)) //MSG_CHANNEL_MAX去除掉一个 

#define MSG_LISTCHN_MAX     (65536U - 20U - 8U) // 最大节目单数据包  20U IP头   8U UDP头
#define MAX_LISTCHN_DATA    (MSG_LISTCHN_MAX - sizeof(chnid_t))


/* 频道包，第一字节描述频道号，data[0]在结构体最后作用为变长数组，根据malloc到的实际内存大小决定 */
typedef struct msg_channel_t{
    chnid_t chnid;             // must between MINCHNID MAXCHNID 频道号（1字节） 取值范围MINCHNID~MAXCHNID
    uint8_t data[0];           // 柔性数组，实际数据存储位置（长度由MAX_DATA决定）
} __attribute__((packed)) msg_channel_t;  //不做对其，因为chnid就只有一个字节， data 通常会很大（这个我还不是很理解，对其的话chnid不是最多也就是4个字节码，感觉有没有这个没多大差）

/* 单个节目信息，第一字节描述频道号，第二三字节为本条信息的总字节数，desc[0]为变长数组 */
typedef struct msg_listdesc_t{
    chnid_t chnid;             // 频道号（1字节）
    uint16_t deslength;        // 自述包长度 描述信息长度
    uint8_t desc[0];           // 柔性数组，存储频道描述文本（UTF-8格式）
} __attribute__((packed)) msg_listdesc_t;


/* 节目单数据包，第一字节描述频道号，list[0]为变长数组，存储msg_listdesc_t变长内容 */
typedef struct msg_list_t{
    chnid_t chnid;           // 频道号（1字节）
    msg_listdesc_t list[0];  // 柔性数组，包含多个节目条目
}__attribute__((packed)) msg_list_t;

#endif
```

## 服务端：

需要完成的任务:

1.将medialib目录下的文件转化为广播频道  

2.广播节目单

3.广播频道内容

整体的复现思路：

1.server_conf.h 里面有一些服务端的基础配置

2.threadpool 这个比较独立，就是基础的线程池，用来管理线程。不依赖于其他模块

3.tokenbucket 令牌桶，用来控制流量，防止流量过大

4.medialib 主要是将medialib目录下的文件转化为广播频道，然后通过list广播节目单，通过channel广播频道内容

5.channel 主要是广播频道内容

6.list 主要是广播节目单

7.server 主要是管理线程池，然后调用list和channel来广播节目单和频道内容

按照这个思路，依次完成各个模块，然后最后整合起来，就可以完成服务端了

### server_conf.h

没啥好说的，直接看代码

```cpp
#ifndef __SERVER_CONF_H__
#define __SERVER_CONF_H__

#define DEFAULT_MEDIADIR "/var/medialib"    // 默认本地媒体库路径
#define DEFAULT_IF "ens33"                  // 默认网卡

#include "threadpool.h"

//运行模式，守护进程会把进程放到后台
enum RNUMODE                                // 运行模式    
{
    RUN_DAEMON = 0,                         // 守护进程
    RUN_FOREGROUND                          // 前台运行
};

typedef struct server_conf_t                // 配置文件,
{
    char *mgroup;                           // [字符串] 组播组IP地址（如："239.0.0.1"）
    char *rcvport;                          // [字符串] 接收端口号（如："8080"）
    char *media_dir;                        // [字符串] 媒体文件存储目录路径
    enum RNUMODE runmode;                   // [字符] 运行模式（使用上述枚举值：RUN_DAEMON/RUN_FOREGROUND）
    char *ifname;                           // [字符串] 网络接口名称（用于绑定组播）
} server_conf_t;

extern server_conf_t server_conf;           // 配置文件
extern int serversd;                        // 服务端套接字
extern struct sockaddr_in sndaddr;          // 发送目的地址
extern ThreadPool_t *pool;                  // 线程池对象

#endif // !__SERVER_CONF_H__
```

### threadpool

这个要说一下，我之前做过webserver的项目，那个里面的线程池比较简单，就是单纯的addtask，然后执行任务就可以了，但是这个项目里面的线程池构造的很巧妙，如果看了代码就可以很好理解。

首先最基本的功能肯定是有的，就是addTask，然后执行任务

这里面还附加了另外的功能，就是设置了一个管理的线程，这个管理的线程用于管理线程池中线程的数量，当任务不多的时候，没必要保存那么多线程，这个时候就可以kill掉一些，当然会有一个最小的数量。当任务很多的时候，就需要额外申请一些线程，但是这个数量也是有限制的。

这样来实现线程数量的动态管理，不会因为空线程而浪费太多的CPU资源

有几个注意的点:

1.NUMSTEP, 每次进行添加和删除线程都会增加或者删除 NUMSTEP 多的线程。不会说一下子减少到最少，或者是一下子添加到最大。拿减少的来举例：如果检测到很多线程空闲，我先减少NUMSTEP个线程，然后再次检测，如果还是空闲，我再减少NUMSTEP多线程。

2.numExit 这个设置的就很巧妙，就是如果我要删除掉一个线程，我就把numExit设置为1，并且notify一下，线程在working (在working函数里面) 的时候，先检测一下numExit是不是1 (这里肯定是要加锁的哈)，如果是1，那么我就不去task，直接退出（自杀）就好了，顺便把numExit--（上锁哈）。是不是感觉很巧妙

3.这里面有个独立出来的manage的线程哈，这个别忘记了，执行的是manager函数，里面就涉及到了之前说的一些线程的增加与删除等操作


代码:

```cpp
//threadpool.h
#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

// #define DEBUG                                // 定义宏，DEBUG模式，打印尽可能多的的log信息，注释则不打印

typedef void ThreadPool_t;                      // 对外隐藏ThreadPool_t内部实现

/*
 * @name            : threadpool_create
 * @description		: 线程池创建函数，创建一个线程池
 * @param - min 	: 最小线程池数
 * @param - max 	: 最大线程池数
 * @param - queueCapacity : 最大任务队列数
 * @return 			: 失败返回 NULL，成功返回线程池对象地址
 */
ThreadPool_t* threadpool_create(int min,int max,int queueCapacity);                     // 新创建一个线程池

/*
 * @name            : threadpool_addtask
 * @description		: 任务队列添加任务函数，添加一个任务
 * @param - argPool : 传入需要添加任务的线程池
 * @param - function: 任务函数 void (*)(void *, volatile int *) 函数指针  【返回类型 (*指针变量名)(参数列表)】
 * @param - arg     : 任务函数参数
 * @return 			: 失败返回 -1，成功返回 0
 */
int threadpool_addtask(ThreadPool_t *, void (*)(void *, volatile int *), void *);       // 向任务队列添加一个任务


/*
 * @name            : threadpool_destroy
 * @description		: 线程池销毁函数，销毁一个线程池
 * @param - argPool : 传入需要销毁的线程池对象
 * @return 			: 失败返回 -1，成功返回 0
 */
int threadpool_destroy(ThreadPool_t *);                                                 // 销毁一个线程池

/*
 * @name            : threadexit_unlock
 * @description		: 线程退出函数，并将该线程 ID 从工作者线程数组中删除
 * @param - argPool : 传入线程池对象
 * @return 			: 无
 */
void threadexit_unlock(ThreadPool_t *);                                                 // 线程退出函数

/*
 * @name            : get_thread_live
 * @description		: 获取线程池中存活线程数
 * @param - argPool : 传入线程池对象
 * @return 			: 线程池中存活线程数
 */
int get_thread_live(ThreadPool_t *);                                                   // 获得线程池中的存活线程数

/*
 * @name            : get_thread_busy
 * @description		: 获取线程池中忙线程数
 * @param - argPool : 传入线程池对象
 * @return 			: 线程池中忙线程数
 */
int get_thread_busy(ThreadPool_t *);                                                   // 获得线程池中的忙线程数
#endif
```

```cpp
// threadpool.cs
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>

#include "threadpool.h"

#define NUMSTEP 5      // 线程增减步长
#define BUFSIZE 1024   // 状态显示缓冲区大小

typedef struct Task_t {
    void (*function)(void *, volatile int *);   // 任务函数指针
    void *arg;                                  // 任务参数
} Task_t;

// 定义线程池类型
struct ThreadPool_t{ 
    // 任务队列相关
    Task_t *taskQueue;          // 任务队列数组
    int queueCapacity;          // 任务队列最大容量
    int queueSize;              // 任务队列当前任务数
    int queueRear;              // 队尾
    int queueFront;             // 队头

    // 线程管理相关
    pthread_t managerID;        // 管理者线程ID
    pthread_t *workerIDs;       // 工作者线程ID数组
    int numMax;                 // 工作者线程最大的线程数
    int numMin;                 // 工作者线程最小的线程数
    int numLive;                // 工作者线程存活的线程数
    int numBusy;                // 工作者线程忙的线程数
    int numExit;                // 工作者线程需要退出的线程数

    // 同步机制
    pthread_mutex_t mutexPool;  // 线程池锁
    pthread_mutex_t mutexBusy;  // 忙线程数锁
    pthread_cond_t notFull;     // 非满条件变量，用于唤醒生产者(添加任务函数)
    pthread_cond_t notEmpty;    // 非空条件变量，用于唤醒消费者(工作者线程)

    volatile int shutstatus;             // 线程池状态，0 打开，-1 关闭
};

// DEBUG 模式下 调试函数：图形化显示线程状态
#ifdef DEBUG
static void printstatus(ThreadPool_t *argPool){
    struct ThreadPool_t *pool = (struct ThreadPool_t *)argPool;
    int numLive, numBusy;
    int i;
    char buf[BUFSIZE];
    memset(buf, 0, BUFSIZE);

    // 获取线程状态
    numLive = get_thread_live(pool);
    numBusy = get_thread_busy(pool);
    // 构建状态字符串：'+'表示忙碌，'-'表示空闲
    for (i = 0; i < numBusy; i++)
        strcat(buf, "+");
    for (i = 0; i < numLive - numBusy; i++)
        strcat(buf, "-");
    // 输出格式：[++++++++++-----] : busy=10, live=15
    fprintf(stdout, "[ %s ] : busy == %d, live == %d\n", buf, numBusy, numLive);
}
#endif // DEBUG

/*
 * @name            : working
 * @description		: 工作者线程任务函数，负责从任务队列中取出任务并执行
 * @param - arg 	: 传入线程池对象
 * @return 			: NULL
 */
static void *working(void *arg){
    struct ThreadPool_t *pool = (struct ThreadPool_t *)arg;
    Task_t task;
    sigset_t set;
    // 阻塞所有信号，避免工作线程处理信号
    sigfillset(&set);
    sigprocmask(SIG_BLOCK, &set, NULL);

    while (1){
        pthread_mutex_lock(&pool->mutexPool);  // 加线程池锁
        while (pool->queueSize == 0 && pool->shutstatus == 0){
            pthread_cond_wait(&pool->notEmpty, &pool->mutexPool); // 阻塞直到任务队列不为空
            // 被唤醒后检查是否需要退出线程
            if (pool->numExit > 0) {
                pool->numExit--;
                if (pool->numLive > pool->numMin){
                    pool->numLive--;
                    pthread_mutex_unlock(&pool->mutexPool);
                    threadexit_unlock(pool); // 执行线程退出流程
                }
            }
        }

        if (pool->shutstatus == -1) // 若线程池已经关闭，线程自杀
        {
            pthread_mutex_unlock(&pool->mutexPool);
            threadexit_unlock(pool);
        }

        // 从队列之中取任务
        task.function = pool->taskQueue[pool->queueFront].function;      // 取出任务
        task.arg = pool->taskQueue[pool->queueFront].arg;
        memset(&pool->taskQueue[pool->queueFront], 0, sizeof(Task_t));   // 从队列取出任务后，将队列中相应任务清空
        pool->queueFront = (pool->queueFront + 1) % pool->queueCapacity; // 移动队头指针
        pool->queueSize--;                                               // 任务队列中任务数量-1
        pthread_cond_signal(&pool->notFull);                             // 唤醒任务生产者 //取出来一个任务后有空位置了自然要处理一下
        pthread_mutex_unlock(&pool->mutexPool);

        pthread_mutex_lock(&pool->mutexBusy); // 加锁，改变线程池中忙线程数
        pool->numBusy++;
        pthread_mutex_unlock(&pool->mutexBusy);
#ifdef DEBUG
        fprintf(stdout, "[thread = %ld] is going to work...\n", pthread_self());
#endif // DEBUG

        task.function(task.arg, &(pool->shutstatus)); // 执行任务

#ifdef DEBUG
        fprintf(stdout, "[thread = %ld] is done work...\n", pthread_self());
#endif // DEBUG
        
        free(task.arg); // 释放任务资源

        syslog(LOG_INFO, "thread [%ld] is free successful...", pthread_self());

#ifdef DEBUG
        fprintf(stdout, "[thread = %ld] is free successful...\n", pthread_self());
#endif // DEBUG

        task.function = NULL;
        task.arg = NULL;

        pthread_mutex_lock(&pool->mutexBusy);
        pool->numBusy--;
        pthread_mutex_unlock(&pool->mutexBusy);
        sched_yield(); // 出让调度器给其他线程
    }
    pthread_exit(NULL);
}


/*
 * @name            : manager
 * @description		: 管理者线程任务函数，负责监视、增加和减少线程池中线程的存活线程数量
 * @param - arg 	: 传入线程池对象
 * @return 			: NULL
 */
static void *manager(void *arg){
    struct ThreadPool_t *pool = (struct ThreadPool_t *)arg;
    struct timeval tv;
    int numLive, numBusy, queueSize;
    int i, count;
    sigset_t set;

    // 阻塞所有信号
    sigfillset(&set);
    sigprocmask(SIG_BLOCK, &set, NULL);

    while (pool->shutstatus == 0) {  // 线程池运行中循环
        // 定时2s，可根据实际场景改变
        tv.tv_sec = 2;
        tv.tv_usec = 500000;
        select(0, NULL, NULL, NULL, &tv);  // select作为延时函数，替换sleep，保证线程安全

        // 获取当前状态（减少锁持有时间）
        pthread_mutex_lock(&pool->mutexPool);
        numLive = pool->numLive;
        queueSize = pool->queueSize;
        pthread_mutex_unlock(&pool->mutexPool);

        pthread_mutex_lock(&pool->mutexBusy);
        numBusy = pool->numBusy;
        pthread_mutex_unlock(&pool->mutexBusy);

        // 动态扩容逻辑 
        count = 0;  
        if ((numLive < queueSize || numBusy > numLive*0.8) && numLive < pool->numMax) { // 当存活线程数小于待取任务数量，并且小于最大线程数
            pthread_mutex_lock(&pool->mutexPool); // 添加 NUMSTEP 步长的线程
            // 以NUMSTEP为步长创建新线程
            for (int i = 0; i < pool->numMax && count < NUMSTEP && pool->numLive < pool->numMax; i++) {
                if (pool->workerIDs[i] == 0) {  // 找到空闲位置
                    pthread_create(&pool->workerIDs[i], NULL, working, pool);
                    count++;
                    pool->numLive++;
                }
            }
            pthread_mutex_unlock(&pool->mutexPool);

        }
        //释放多余线程
        if (numBusy * 2 < numLive && numLive > pool->numMin) { // 当忙线程数 * 2小于存活线程数，并且存活的线程大于最小线程数
            pthread_mutex_lock(&pool->mutexPool);
            pool->numExit = NUMSTEP;  // 设置退出数量
            pthread_mutex_unlock(&pool->mutexPool);
            for (i = 0; i < NUMSTEP; i++){
                pthread_cond_signal(&pool->notEmpty); // 唤醒工作线程，让其自杀
            }
        }

#ifdef DEBUG
        printstatus(pool); // 打印线程池中线程信息
#endif // DEBUG
        sched_yield();     // 出让调度器  
    }
    pthread_exit(NULL);
}

/*
 * @name            : threadpool_create
 * @description		: 线程池创建函数，创建一个线程池
 * @param - min 	: 最小线程池数
 * @param - max 	: 最大线程池数
 * @param - queueCapacity : 最大任务队列数
 * @return 			: 失败返回 NULL，成功返回线程池对象地址
 */
ThreadPool_t *threadpool_create(int min, int max, int queueCapacity){
    struct ThreadPool_t* pool = malloc(sizeof(struct ThreadPool_t));
    int i;
    do{
        if(pool == NULL){
            syslog(LOG_ERR, "threadpool malloc() : %s", strerror(errno));
            break; // 申请内存失败就跳过剩下的初始化
        }
        // 初始化任务队列
        pool->taskQueue = malloc(sizeof(Task_t) * queueCapacity);
        if (pool->taskQueue == NULL){
            syslog(LOG_ERR, "taskQueue malloc() : %s", strerror(errno));
            break;
        }
        memset(pool->taskQueue, 0, sizeof(Task_t) * queueCapacity); //清空一下
        pool->queueCapacity = queueCapacity; // 各个成员的初始化
        pool->queueSize = 0;
        pool->queueRear = 0;
        pool->queueFront = 0;

        // 初始化工作线程数组
        pool->workerIDs = malloc(sizeof(pthread_t) * max);
        if (pool->workerIDs == NULL)
        {
            syslog(LOG_ERR, "workerIDs malloc() : %s", strerror(errno));
            break;
        }
        memset(pool->workerIDs, 0, sizeof(pthread_t) * max); //清空一下
        pool->numMax = max;
        pool->numMin = min;
        pool->numLive = min;
        pool->numBusy = 0;
        pool->numExit = 0;

        if (pthread_mutex_init(&pool->mutexPool, NULL) != 0 || // 初始化锁和条件变量
            pthread_mutex_init(&pool->mutexBusy, NULL) != 0 ||
            pthread_cond_init(&pool->notFull, NULL) != 0 ||
            pthread_cond_init(&pool->notEmpty, NULL) != 0)
        {
            syslog(LOG_ERR, "lock init failed ...");
            break;
        }

        pool->shutstatus = 0;  // 运行状态 // 开启线程池

        pthread_create(&pool->managerID, NULL, manager, pool); // 创建管理者线程

        for(i=0;i<min;i++){
            if (pool->workerIDs[i] == 0){
                pthread_create(&pool->workerIDs[i], NULL, working, pool); // 创建工作者线程
            }
        }

        return pool;

    } while (0);

    if (pool != NULL && pool->workerIDs != NULL) // 申请内存失败跳转到这里开始，依次析构 //这边是错误处理
    free(pool->workerIDs);
    if (pool != NULL && pool->taskQueue != NULL)
        free(pool->taskQueue);
    if (pool != NULL)
        free(pool);
    return NULL;
    
}


/*
 * @name            : threadpool_destroy
 * @description		: 线程池销毁函数，销毁一个线程池
 * @param - argPool : 传入需要销毁的线程池对象
 * @return 			: 失败返回 -1，成功返回 0
 */
int threadpool_destroy(ThreadPool_t *argPool){
    int i;
    struct ThreadPool_t *pool = (struct ThreadPool_t *)argPool;

    if (pool == NULL)
    {
        syslog(LOG_ERR, "thread pool is not existed ...");
        return -1;
    }

    pool->shutstatus = -1;  // 触发关闭

    sleep(2);
    pthread_join(pool->managerID, NULL); // join掉managerID线程

    for (i = 0; i < pool->numLive; i++)
    {
        pthread_cond_signal(&pool->notEmpty); // 唤醒所有存活线程，让其自杀
    }
    //信号量销毁
    pthread_mutex_destroy(&pool->mutexBusy);
    pthread_mutex_destroy(&pool->mutexPool);
    pthread_cond_destroy(&pool->notEmpty);
    pthread_cond_destroy(&pool->notFull);

    // free掉空间
    if (pool != NULL && pool->workerIDs != NULL)
        free(pool->workerIDs);
    if (pool != NULL && pool->taskQueue != NULL)
        free(pool->taskQueue);
    if (pool != NULL)
        free(pool);
    pool = NULL;

    syslog(LOG_INFO, "thread pool is going to be destroyed...");
#ifdef DEBUG
    fprintf(stdout, "thread pool is going to be destroyed...\n");
#endif // DEBUG
    return 0;
}

/*
 * @name            : threadpool_addtask
 * @description		: 任务队列添加任务函数，添加一个任务
 * @param - argPool : 传入需要添加任务的线程池
 * @param - function: 任务函数
 * @param - arg     : 任务函数参数
 * @return 			: 失败返回 -1，成功返回 0
 */
int threadpool_addtask(ThreadPool_t *argPool, void (*function)(void *, volatile int*), void *arg){
    struct ThreadPool_t *pool = (struct ThreadPool_t *)argPool;
    pthread_mutex_lock(&pool->mutexPool);
    while(pool->queueSize == pool->queueCapacity && pool->shutstatus == 0){
        pthread_cond_wait(&pool->notFull, &pool->mutexPool); // 阻塞直到等待任务队列不为满
    }
    if(pool->shutstatus == -1){
        syslog(LOG_INFO, "thread pool has been shutdown ...");
        pthread_mutex_unlock(&pool->mutexPool);
        return -1;
    }

    pool->taskQueue[pool->queueRear].function = function; // 将任务存储到任务队列中
    pool->taskQueue[pool->queueRear].arg = arg;
    pool->queueRear = (pool->queueRear + 1) % pool->queueCapacity; // 移动队尾指针
    pool->queueSize++;
    pthread_cond_signal(&pool->notEmpty); // 队列不为空，唤醒工作者线程
    pthread_mutex_unlock(&pool->mutexPool);
    return 0;
}


/*
 * @name            : threadexit_unlock
 * @description		: 线程退出函数，并将该线程 ID 从工作者线程数组中删除
 * @param - argPool : 传入线程池对象
 * @return 			: 无
 */
void threadexit_unlock(ThreadPool_t *argPool){
    int i;
    struct ThreadPool_t *pool = (struct ThreadPool_t *)argPool;
    pthread_t tmptid = pthread_self();
    for (i = 0; i < pool->numMax; i++)
    {
        if (pool->workerIDs[i] == tmptid)
        {
            pool->workerIDs[i] = 0;
            break;
        }
    }
    syslog(LOG_INFO, "thread [%ld] is going to exit...", tmptid);
#ifdef DEBUG
    fprintf(stdout, "[thread = %ld] is going to exit...\n", tmptid);
#endif // DEBUG
    pthread_exit(NULL);
}

/*
 * @name            : get_thread_live
 * @description		: 获取线程池中存活线程数
 * @param - argPool : 传入线程池对象
 * @return 			: 线程池中存活线程数
 */
int get_thread_live(ThreadPool_t *argPool)
{
    struct ThreadPool_t *pool = (struct ThreadPool_t *)argPool;
    int num;
    pthread_mutex_lock(&pool->mutexPool);
    num = pool->numLive;
    pthread_mutex_unlock(&pool->mutexPool);
    return num;
}

/*
 * @name            : get_thread_busy
 * @description		: 获取线程池中忙线程数
 * @param - argPool : 传入线程池对象
 * @return 			: 线程池中忙线程数
 */
int get_thread_busy(ThreadPool_t *argPool)
{
    struct ThreadPool_t *pool = (struct ThreadPool_t *)argPool;
    int num;
    pthread_mutex_lock(&pool->mutexBusy);
    num = pool->numBusy;
    pthread_mutex_unlock(&pool->mutexBusy);
    return num;
}

```


### tokenbucket

令牌桶，涉及到一些流量控制，就是我们在读mp3媒体文件的时候，不是要传递一个读取多少字节的参数吗？ 这个时候咱们先从令牌桶里面取令牌，如果说令牌数量很充足，就是大于size (size是咱们想要读的字节) 的话就取出size个令牌，然后读size个字节。但是如果说令牌桶里面取令牌不足size，比如只有size1这么多了，那么咱们读数据的时候就取出size1这么多字节。

这里面有个计时器，每一秒会使得令牌桶里面的令牌的数量增加

```cpp
struct tokenbt_t{
    int cps;                    // 步长
    int burst;                  // 上限
    int token;                  // 拥有的令牌
    int pos;                    // 自述数组下标 在全局job数组中的位置索引,不是只有一个桶
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};
```
这里看一下单个令牌桶的结构

注意的点：

1.这个定时器是额外定义的线程，他不会占用主线程。

2.并不是只有一个令牌桶，这里设计了一个令牌桶的数组 job数组。里面存储了好多令牌桶，这样针对每个频道，我们都可以为其分配一个令牌桶。让每个频道独立起来。而不是所有的频道公用一个令牌桶。

3.注意这个令牌桶使用的位置哈，并不是sendto和recvfrom这两个函数用，而是在读取mp3文件的时候来用，就是通过控制文件读取的速度来控制sendto发送数据包的速度。

4.这里有两种类型的锁，一个是job队列的锁，就是对job队列进行操作的时候必须得加锁。还有一个是桶内部的锁，看上面的结构，是不是每个桶都有一个锁，这是为了在实现安全的修改每个桶里面的数据。

```cpp
#ifndef __TOKENBUCKET_h__
#define __TOKENBUCKET_h__

#define TOKENBUCKET_MAX 1024    //最大令牌桐对象数量

typedef void tokenbt_t;         //对外隐藏内部实现细节

/*
 * @name            : tokenbt_init
 * @description		: 令牌桶初始化，初始化一个令牌桶对象
 * @param - cps     : 步长 每一s增加多少令牌数量
 * @param - burst   : 上限零牌数
 * @return 			: 成功返回令牌桶对象，失败返回 NULL
 */
tokenbt_t *tokenbt_init(int, int);

/*
 * @name            : tokenbt_fetchtoken
 * @description		: 从令牌桶对象中取令牌
 * @param - token   : 令牌桶对象
 * @param - size    : 要取的零牌数
 * @return 			: 成功返回令牌，失败返回 -EINVAL
 */
int tokenbt_fetchtoken(tokenbt_t *, int);  


/*
 * @name            : tokenbt_checktoken
 * @description		: 检查令牌桶对象的令牌数
 * @param - token   : 令牌桶对象
 * @return 			: 成功返回令牌，失败返回 -EINVAL
 */
int tokenbt_checktoken(tokenbt_t *);

/*
 * @name            : tokenbt_returntoken
 * @description		: 给令牌桶对象归还令牌
 * @param - token   : 令牌桶对象
 * @param - size    : 令牌
 * @return 			: 成功返回归还的令牌，失败返回 -EINVAL
 */
int tokenbt_returntoken(tokenbt_t *, int);

/*
 * @name            : tokenbt_destroy
 * @description		: 销毁单个令牌桶对象
 * @param - token   : 令牌桶对象
 * @return 			: 成功返回 0，失败返回 -EINVAL
 */
int tokenbt_destroy(tokenbt_t *); 

/*
 * @name            : tokenbt_destroy_all
 * @description		: 销毁所有令牌桶对象
 * @param           : 无
 * @return 			: 成功返回 0
 */
int tokenbt_destroy_all(); 

/*
 * @name            : tokenbt_shutdown
 * @description		: 关闭令牌流控功能模块
 * @param           : 无
 * @return 			: 成功返回 0
 */
int tokenbt_shutdown(); 



#endif // !__TOKENBUCKET_h__
```

```cpp
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>

#include "tokenbucket.h"

struct tokenbt_t{
    int cps;                    // 步长
    int burst;                  // 上限
    int token;                  // 拥有的令牌
    int pos;                    // 自述数组下标 在全局job数组中的位置索引,不是只有一个桶
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

//现在有TOKENBUCKET_MAX个桶
static struct tokenbt_t *token_pool[TOKENBUCKET_MAX];

// 保护job数组的互斥锁，防止多线程并发修改
static pthread_mutex_t mutex_pool = PTHREAD_MUTEX_INITIALIZER; 
// 确保init模块只会加载一次
static pthread_once_t once_init = PTHREAD_ONCE_INIT;
// 定时器的线程ID
static pthread_t tid;

/*
 * @name            : woking
 * @description		: 任务函数，负责固定时间给令牌桶添加令牌数
 * @param - arg     : NULL
 * @return 			: NULL
 */
static void *woking(void *arg){
    int i;
    struct timeval tv;
    sigset_t set;

    sigfillset(&set);
    sigprocmask(SIG_BLOCK, &set, NULL);     // 屏蔽所有信号，只留主线程处理信号即可

    while(1){
        pthread_mutex_lock(&mutex_pool); //对token_pool加锁
        for(i=0;i<TOKENBUCKET_MAX;i++){
            if(token_pool[i]!=NULL){
                pthread_mutex_lock(&token_pool[i]->mutex);
                token_pool[i]->token += token_pool[i]->cps;
                if (token_pool[i]->token > token_pool[i]->burst){ //已经到最大值了
                    token_pool[i]->token = token_pool[i]->burst;
                }
                pthread_cond_broadcast(&token_pool[i]->cond); //notifyAll
                pthread_mutex_unlock(&token_pool[i]->mutex);
            }
        }
        pthread_mutex_unlock(&mutex_pool);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        select(0, NULL, NULL, NULL, &tv);       // 安全定时 1 s
    }
    pthread_exit(NULL);
}

/*
 * @name            : get_free_pos_unlocked
 * @description		: 任务队列添加任务函数，添加一个任务
 * @param           : 无
 * @return 			: 失败返回 -1，成功返回 找到的空闲数组位置
 */
static int get_free_pos_unlocked(){  //找到token_pool之中第一个空闲值
    int i;
    for(i =0;i<TOKENBUCKET_MAX;i++){
        if(token_pool[i]==NULL){
            return i;
        }
    }
    return -1;
}

/*
 * @name            : module_unload
 * @description		: 令牌桶模块卸载
 * @param           : 无
 * @return 			: 无
 */
static void module_unload(){
    pthread_cancel(tid);        // 取消任务函数线程(负责固定时间给令牌桶添加令牌数)
    pthread_join(tid, NULL);
    tokenbt_destroy_all();
    syslog(LOG_INFO, "free job is done ...");
}

/*
 * @name            : module_load
 * @description		: 令牌桶模块加载
 * @param           : 无
 * @return 			: 无
 */
static void module_load(){
    pthread_create(&tid, NULL, woking, NULL);  //创建计时器线程
}

/*
 * @name            : tokenbt_init
 * @description		: 令牌桶初始化，初始化一个令牌桶对象
 * @param - cps     : 步长
 * @param - burst   : 上限零牌数
 * @return 			: 成功返回令牌桶对象，失败返回 NULL
 */
tokenbt_t *tokenbt_init(int cps, int burst){
    struct tokenbt_t* tb;
    int pos;
    pthread_once(&once_init, module_load);  // 任务函数只执行一次
    tb = malloc(sizeof(struct tokenbt_t));  // 分配空间
    if(tb == NULL){
        return NULL; //错误处理
    }
    tb->cps = cps;    //1s增加多少令牌数
    tb->burst = burst;//令牌数最大值
    tb->token = 0;    //初始令牌数
    pthread_mutex_init(&tb->mutex, NULL);
    pthread_cond_init(&tb->cond, NULL);
    //加锁，要处理tb里面的值了
    pthread_mutex_lock(&mutex_pool);
    pos = get_free_pos_unlocked();
    // 没有空余的空间了
    if (pos < 0)
    {
        pthread_mutex_unlock(&mutex_pool);      // 获取是失败别忘了解锁
        pthread_mutex_destroy(&tb->mutex);
        pthread_cond_destroy(&tb->cond);
        free(tb);
        syslog(LOG_ERR, "have not any pool pos...");
        // fprintf(stderr, "have not any pool pos...\n");
        return NULL;
    }
    //把tb插入到工作队之中
    tb->pos = pos;
    token_pool[pos] = tb;
    pthread_mutex_unlock(&mutex_pool);
    return tb;
}

/*
 * @name            : tokenbt_fetchtoken
 * @description		: 从令牌桶对象中取令牌
 * @param - token   : 令牌桶对象
 * @param - size    : 要取的零牌数
 * @return 			: 成功返回令牌，失败返回 -EINVAL
 */
int tokenbt_fetchtoken(tokenbt_t *token, int size){
    struct tokenbt_t *tb = (struct tokenbt_t *)token;
    int n;
    if (size <= 0 || token == NULL)     // 判断参数是否合法
        return -EINVAL;
    //每一次操作tb内部的东西都要加锁
    pthread_mutex_lock(&tb->mutex);
    while (tb->token <= 0){  //没有空余的token了，就等待
        pthread_cond_wait(&tb->cond, &tb->mutex);
    }
    n = tb->token;
    n = n < size ? n : size;        // 取得零牌数较小的那一个
    tb->token -= n;
    pthread_mutex_unlock(&tb->mutex);
    return n;   //返回的并不是size，而是实际取到的令牌数
}

/*
 * @name            : tokenbt_checktoken
 * @description		: 检查令牌桶对象的令牌数
 * @param - token   : 令牌桶对象
 * @return 			: 成功返回令牌，失败返回 -EINVAL
 */
int tokenbt_checktoken(tokenbt_t *token){
    struct tokenbt_t *tb = (struct tokenbt_t *)token;
    int token_size;
    if(tb == NULL){ //错误处理
        return -EINVAL;
    }
    pthread_mutex_lock(&tb->mutex);
    token_size = tb->token;
    pthread_mutex_unlock(&tb->mutex);
    return token_size;
}

/*
 * @name            : tokenbt_returntoken
 * @description		: 给令牌桶对象归还令牌
 * @param - token   : 令牌桶对象
 * @param - size    : 令牌
 * @return 			: 成功返回归还的令牌，失败返回 -EINVAL
 */
int tokenbt_returntoken(tokenbt_t *token, int size){
    struct tokenbt_t *tb = (struct tokenbt_t *)token;
    if (size <= 0 || token == NULL){ //参数正常
        return -EINVAL;
    }
    pthread_mutex_lock(&tb->mutex);
    tb->token += size;
    if (tb->token > tb->burst)
        tb->token = tb->burst;
    pthread_cond_broadcast(&tb->cond);
    pthread_mutex_unlock(&tb->mutex);
    return size;
}

/*
 * @name            : tokenbt_destroy
 * @description		: 销毁单个令牌桶对象
 * @param - token   : 令牌桶对象
 * @return 			: 成功返回 0，失败返回 -EINVAL
 */
int tokenbt_destroy(tokenbt_t *token){
    struct tokenbt_t *tb = (struct tokenbt_t *)token;
    if (token == NULL)
    return -EINVAL;
    pthread_mutex_lock(&mutex_pool);
    token_pool[tb->pos] = NULL;        //要把这个令牌桶从工作队列中拿出去
    pthread_mutex_unlock(&mutex_pool);
    // 然后再释放掉这个令牌桶内部的空间
    pthread_mutex_destroy(&tb->mutex);
    pthread_cond_destroy(&tb->cond);
    free(tb);
    tb = NULL;
    return 0;
}


/*
 * @name            : tokenbt_destroy_all
 * @description		: 销毁所有令牌桶对象
 * @param           : 无
 * @return 			: 成功返回 0
 */
int tokenbt_destroy_all(){
    for(int i=0;i<TOKENBUCKET_MAX;i++){
        if (token_pool[i] != NULL)
        {
            pthread_mutex_destroy(&token_pool[i]->mutex);
            pthread_cond_destroy(&token_pool[i]->cond);
        }
        free(token_pool[i]);
        token_pool[i] = NULL;
    }
    return 0;
}

/*
 * @name            : tokenbt_shutdown
 * @description		: 关闭令牌流控功能模块
 * @param           : 无
 * @return 			: 成功返回 0
 */
int tokenbt_shutdown()
{
    module_unload();
    return 0;
}
```

### medialib

这个的任务就是将我们本地的medialib文件夹里面的内容转化为广播频道

主要完成两项任务：

1.节目单的组合(mlib_getchnlist函数)

2.读取频道内部的数据（mlib_readchn函数）

这两个函数是最关键的，之前所说的令牌桶就是在mlib_readchn函数里面用的，就是读取mp3文件的操作

首先我们来说一下结构

```
├── medialib
│   ├── ch1
│   │   ├── desc.txt
│   │   ├── 世界这么大还是遇见你.mp3
│   │   └── 起风了.mp3
│   ├── ch2
│   │   ├── desc.txt
│   │   ├── 带你去旅行.mp3
│   │   └── 再见只是陌生人.mp3
│   ├── ch3
│   │   ├── desc.txt
│   │   ├── Summertime Sadness.mp3
│   │   └── 学猫叫.mp3
│   └── ch4
```
每个ch之中都会有一个desc.txt的文件，这个里面就写的一些频道的基本的描述，比如:

ch1中内容就一行: pop music,起风了,世界这么大还是遇见你

所以我们要获取一下这个节目单的描述啥的还是很简单的，就是读取desc.txt文件就行了

我们看一下每个频道的管理的结构体

```cpp

typedef struct channel_context_t {   //频道内容描述结构体
    chnid_t chnid;   // 频道ID
    char* desc;      // 频道描述
    glob_t globes;   // 目录项
    int pos;         // 当前歌曲在媒体库中的位置
    int fd;          // 当前歌曲的文件描述符
    off_t offset;    // 当前歌曲的读取偏移量
    tokenbt_t *tb;   // 流量控制
} channel_context_t;

// typedef struct {
//   size_t   gl_pathc;    /* 匹配到的路径数目 */
//   char   **gl_pathv;    /* 匹配到的路径名指针数组 */ 是一个指向匹配到的路径名字符串数组的指针。数组中的每个元素是一个路径名字符串，最后一个元素为 NULL，表示数组结束。
//   size_t   gl_offs;     /* 起始位置索引 */ 表示起始位置索引。如果设置了 GLOB_DOOFFS 标志，glob() 函数会在 gl_pathv 的开头预留 gl_offs 个空指针，实际的匹配路径名从第 gl_offs 个位置开始。
// } glob_t;
```

这里主要是几个参数可能不好理解

1.glob_t globes 这个主要是用来搜索一下*mp3的内容的，搜索出来的文件路径会存在结构体的gl_pathv里面

2.pos 就是因为会有很多.mp3文件，这个就标志着这是第几个文件，从0开始，这样的话方便列表循环播放

3.fd 这个就是目前打开的mp3的文件描述符，用以后面读文件

4.offset 这个文件读到哪里了，用这个来表示，方便下次继续读

5.tokenbt_t *tb;   令牌桶就用在这里

代码：

```cpp
#ifndef __MEDIALIB_H__
#define __MEDIALIB_H__

#include <protocol.h>
#include <unistd.h>

//记录每一条节目单信息：频道号chnid，描述信息char* desc
typedef struct mlib_listdesc_t{
    chnid_t chnid;  // 频道唯一标识符，使用prot.h中定义的chnid_t类型
    char* desc;     // 频道描述信息，动态分配的字符串指针
}mlib_listdesc_t;

/**
 * 获取频道列表
 * @param  [输出参数] 接收频道信息结构体数组指针的地址
 * @param  [输出参数] 接收频道数量的指针
 * @return  成功返回0，失败返回-1
 */
int mlib_getchnlist(struct mlib_listdesc_t **,int *); // 从媒体库获取节目单信息和频道总个数

/**
 * 释放频道列表内存
 * @param  要释放的频道信息结构体数组指针
 * @return 总是返回0表示成功
 */
int mlib_freechnlist(struct mlib_listdesc_t *);       // 释放节目单信息存储所占的内存


/**
 * 释放chn_context数组的内存 
 * chn_context里面存储的是所有的广播频道号
 * @return 总是返回0表示成功
 */
int mlib_freechncontext();                            // 释放chn_context数组的内存


/*
 * @name            : mlib_readchn
 * @description		: 按频道读取对应媒体库流媒体内容
 * @param - chnid   : 频道号
 * @param - buf     : 存入流媒体内容的缓冲区指针
 * @param - size    : buf的最大容量
 * @return 			: 返回读取到的有效内容总长度
 */
ssize_t mlib_readchn(chnid_t, void *, size_t);        // 按频道读取对应媒体库流媒体内容


#endif // !__MEDIALIB_H__
```

```cpp
#include <protocol.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <glob.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "medialib.h"
#include "server_conf.h"
#include "tokenbucket.h"

#define PATHSIZE 4096      // 文件路径最大长度
#define NAMESIZE 256       // 文件名最大长度
#define LINEBUFSIZE 1024   // 读文件行缓冲区

#define MP3_BITRATE (128 * 1024)    // 128 * 1024 bps

typedef struct channel_context_t {   //频道内容描述结构体
    chnid_t chnid;   // 频道ID
    char* desc;      // 频道描述
    glob_t globes;   // 目录项
    int pos;         // 当前歌曲在媒体库中的位置
    int fd;          // 当前歌曲的文件描述符
    off_t offset;    // 当前歌曲的读取偏移量
    tokenbt_t *tb;   // 流量控制
} channel_context_t;

// typedef struct {
//   size_t   gl_pathc;    /* 匹配到的路径数目 */
//   char   **gl_pathv;    /* 匹配到的路径名指针数组 */ 是一个指向匹配到的路径名字符串数组的指针。数组中的每个元素是一个路径名字符串，最后一个元素为 NULL，表示数组结束。
//   size_t   gl_offs;     /* 起始位置索引 */ 表示起始位置索引。如果设置了 GLOB_DOOFFS 标志，glob() 函数会在 gl_pathv 的开头预留 gl_offs 个空指针，实际的匹配路径名从第 gl_offs 个位置开始。
// } glob_t;


static channel_context_t chn_context[MAXCHNID + 1];
static int total_chn = 0;           // 总共的有效频道个数


/*
 * @name            : getpathcontent
 * @description		: 从指定的路径中取得该频道所有需要的信息
 * @param - path    : 文件路径
 * @return 			: 成功返回 channel_context_t 对象地址; 失败返回 NULL
 */
static channel_context_t *getpathcontent(const char *path){
    char linebuf[LINEBUFSIZE];    //行缓冲
    char pathbuf[PATHSIZE];       //文件路径
    char namebuf[NAMESIZE];       //文件名

    int descfd, ret;
    channel_context_t *me;
    //由于是一个静态变量所以相当于一直在操作同一块内存 有叠加效果  静态频道ID计数器，初始为最小频道ID
    static int curr_chnid = MINCHNID;

    //先清空一下值
    memset(linebuf, 0, sizeof(linebuf));
    memset(pathbuf, 0, sizeof(pathbuf));
    memset(namebuf, 0, sizeof(namebuf));

    //赋值，字符串操作
    strncpy(pathbuf, path, PATHSIZE - 1);                         //path
    strncpy(namebuf, "/desc.txt", NAMESIZE - 1);                  //desc name
    strncat(pathbuf, namebuf, PATHSIZE - strlen(pathbuf) - 1);    //完整path

    descfd = open(pathbuf, O_RDONLY);
    if (descfd < 0){   //错误处理
        syslog(LOG_INFO, "%s is not a lib ...", pathbuf);
        return NULL;
    }

    ret = read(descfd, linebuf, LINEBUFSIZE);  //读文件
    if (ret == 0){ //错误处理
        syslog(LOG_INFO, "%s haven't anything ...", pathbuf);
        close(descfd);
        return NULL;
    }
    close(descfd);  //这里要注意哈，desc文件里面本来就只有一行
    // 申请空间
    me = malloc(sizeof(channel_context_t));
    if(me == NULL){ // 错误处理
        syslog(LOG_ERR, "malloc() : %s", strerror(errno));
        return NULL;
    }
    me->desc = strdup(linebuf); //strdup 复制字符串
    // 令牌桶 流量控制
    me->tb = tokenbt_init(MP3_BITRATE / 8, MP3_BITRATE / 8 * 5);
    if (me->tb == NULL){
        syslog(LOG_ERR, "tokenbt_init() : failed ...");
        free(me);
        return NULL;
    }
    // 这个时候已经把 desc.txt里的内容拿到了，现在该匹配mp3了
    memset(pathbuf, 0, sizeof(pathbuf));
    memset(namebuf, 0, sizeof(namebuf));
    // 一样的操作，path name path+name 只不过这里的是*.mp3
    strncpy(pathbuf, path, PATHSIZE - 1);
    strncpy(namebuf, "/*.mp3", NAMESIZE - 1);
    strncat(pathbuf, namebuf, PATHSIZE - strlen(pathbuf) - 1);

    ret = glob(pathbuf, 0, NULL, &me->globes);  //glob 匹配.mp3文件
    if (ret != 0){
        syslog(LOG_ERR, "glob() : failed ...");
        free(me);
        return NULL;
    }

    me->pos = 0;     //第1个pos，第一个mp3
    me->offset = 0;  //还没开始读，所以就offset（文件内部偏移）就是0

    //打开第一个mp3文件
    me->fd = open(me->globes.gl_pathv[me->pos], O_RDONLY);
    if (me->fd < 0){
        syslog(LOG_ERR, "open() : %s", strerror(errno));
        free(me);
        return NULL;
    }
    // 记录当前的 curr_chnid
    me->chnid = curr_chnid;
    curr_chnid++;
    return me;
}


/*
 * @name            : open_next
 * @description		: 打开指定频道的下一首歌曲
 * @param - chnid   : 频道号
 * @return 			: 成功返回 0; 失败返回 -1
 */
static int open_next(chnid_t chnid){
    int i;
    for (i = 0; i < chn_context[chnid].globes.gl_pathc; i++){
        chn_context[chnid].pos++;
        if (chn_context[chnid].pos == chn_context[chnid].globes.gl_pathc){
            chn_context[chnid].pos = 0;
        } // 列表循环
        close(chn_context[chnid].fd); //先关闭一下当前的
        // 打开下一首
        chn_context[chnid].fd = open(chn_context[chnid].globes.gl_pathv[chn_context[chnid].pos], O_RDONLY);
        if (chn_context[chnid].fd < 0){
            continue; //打不开倒不至于直接退出，再继续循环打开下一首
        }else{
            return 0;
        }
    }
    return -1;
}

/*
 * @name            : mlib_getchnlist
 * @description		: 从媒体库获取节目单信息和频道总个数
 * @param - list    : 传出参数，填入节目单信息
 * @param - size    : 传出参数，填入频道总个数
 * @return 			: 成功返回 0; 失败返回 -1
 */
int mlib_getchnlist(mlib_listdesc_t **list, int *size){
    int i,ret;
    glob_t globes;
    char path[PATHSIZE];
    channel_context_t *retmp;
    mlib_listdesc_t *tmp;

    memset(chn_context, 0, sizeof(chn_context));
    //初始化频道数组 
    for (i = MINCHNID; i < MAXCHNID + 1; i++){
        chn_context[i].chnid = -1; //设置为无效频道
    }
    //拿到medialib path  /*表示medialib下面所有的文件夹(ch1 ch2 ch3 ch4....)
    snprintf(path, PATHSIZE, "%s/*", server_conf.media_dir);

    ret = glob(path, 0, NULL, &globes);  //匹配一下
    if (ret != 0){
        syslog(LOG_ERR, "glob() : failed ...");
        return -1;
    }
    //分配一下空间
    tmp = malloc(sizeof(mlib_listdesc_t) * globes.gl_pathc);
    if (tmp == NULL){
        syslog(LOG_ERR, "malloc() : %s", strerror(errno));
        return -1;
    }

    for (i = 0; i < globes.gl_pathc; i++){       // 分别获取 ch1 ch2 ch3 ch4 中的频道内容并保存在 chn_context 中
        retmp = getpathcontent(globes.gl_pathv[i]);
        if (retmp != NULL){
            //注意这里会把retmp放到全局静态的chn_context之中
            memcpy(chn_context + retmp->chnid, retmp, sizeof(*retmp));
            tmp[total_chn].chnid = retmp->chnid;
            tmp[total_chn].desc = strdup(retmp->desc);
            total_chn++;
            free(retmp);
        }
    }
    *list = realloc(tmp, sizeof(mlib_listdesc_t) * total_chn);      // 给 *list 重新分配内存
    if (list == NULL){
        syslog(LOG_ERR, "realloc() : %s", strerror(errno));
        return -1;
    }
    *size = total_chn;
    globfree(&globes);
    return 0;
}

/*
 * @name            : mlib_freechnlist
 * @description		: 释放节目单信息存储所占的内存
 * @param - list    : 
 * @return 			: 成功返回 0
 */
int mlib_freechnlist(struct mlib_listdesc_t *list){
    int i;
    for (i = 0; i < total_chn; i++)
    {
        free(list[i].desc);
    }
    free(list);
    return 0;
}

/*
 * @name            : mlib_freechncontext
 * @description		: 释放chn_context数组的内存
 * @return 			: 成功返回 0
 */
int mlib_freechncontext(){
    int i;
    for (i = MINCHNID; i < MAXCHNID + 1; i++)
    {
        if (chn_context[i].chnid != -1)
        {
            free(chn_context[i].desc);
            globfree(&chn_context[i].globes);
            close(chn_context[i].fd);
        }
    }
    return 0;
}

/*
 * @name            : mlib_readchn
 * @description		: 按频道读取对应媒体库流媒体内容
 * @param - chnid   : 频道号
 * @param - buf     : 存入流媒体内容的缓冲区指针
 * @param - size    : buf的最大容量
 * @return 			: 返回读取到的有效内容总长度
 */
ssize_t mlib_readchn(chnid_t chnid, void *buf, size_t size){
    int token, len;
    //获取一下令牌桶之中的令牌，返回的是实际获取的令牌数量，因为可能不够size这么多令牌
    token = tokenbt_fetchtoken(chn_context[chnid].tb, size);
    while(1){
        //  从指定的偏移量处开始读取 read的话是从当前文件偏移量开始读取(当前文件偏移量存储再文件描述符之中)
        //  pread 1是可以指定偏移两 2是线程安全，不会引文其他线程读取了文件改变了文件偏移
        len = pread(chn_context[chnid].fd, buf, token, chn_context[chnid].offset);
        if (len < 0){
            if (errno == EINTR)
                return 0;
            syslog(LOG_ERR, "pread() : %s", strerror(errno));
            open_next(chnid);       // 如果这首歌曲读取失败了，那就切换下一首歌曲播放
        }else if(len == 0){
            syslog(LOG_INFO, "song: %s is over", chn_context[chnid].globes.gl_pathv[chn_context[chnid].pos]);
            open_next(chnid);       // 这首歌曲读取结束了，那就切换下一首歌曲播放
            break;
        }else{
            chn_context[chnid].offset += len;  //正常读取了就加offset
            break;
        }
    }
    if (len < token)                // 令牌没用完，归还令牌
    {
        tokenbt_returntoken(chn_context[chnid].tb, token - len);
    }
    return len;
}
```


### list

之前不是medialib把咱么medialib文件夹里面的东西转化为频道了吗

这里就是组装一下，节目单频道，用来发送节目单的数据，代码很简单，看一下就好。

就是，拿到组装号的 节目点list，然后处理一下，然后发送就好。

这里的发送时调用的线程池（addtask）

就是一直发，1s发一次。另外频道号之前在protocol.h已经定义好了，频道列表的频道号固定是0

```cpp
#ifndef __LIST_H__
#define __LIST_H__
#include "medialib.h"

int thr_list_create(mlib_listdesc_t *, int);   // 节目单频道任务创建

#endif // !__LIST_H__
```

```cpp
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "threadpool.h"
#include "server_conf.h"
#include "list.h"

// 将需要发送的节目信息内容和内容的长度打包，传给线程池任务
typedef struct send_list_t
{
    int len;
    msg_list_t msg[0];
} send_list_t;

/*
 * @name            : sendlist
 * @description		: 定时发送节目单信息
 * @param - arg     : send_list_t类型
 * @param - shut    : 线程池当前开启状态
 * @return 			: 无
 */
void sendlist(void *arg, volatile int *shut){
    int len;
    send_list_t *info = (send_list_t *)arg;
    while (*shut == 0)
    {
        len = sendto(serversd, info->msg, info->len, 0, (void *)&sndaddr, sizeof(sndaddr));
        syslog(LOG_INFO, "%7s thread sendto %5d bytes, pool status is %d", "list", len, *shut);
        sleep(1);
    }
}

/*
 * @name            : thr_list_create
 * @description		: 节目单频道任务创建
 * @param - list    : 从媒体库读取的原始节目信息数据
 * @param - size    : 频道总个数
 * @return 			: 成功返回 0; 失败返回 -1
 */
int thr_list_create(mlib_listdesc_t *list, int size){
    int len, totalsize;
    int i;
    msg_list_t *msg_list;
    msg_listdesc_t *desc_list;
    send_list_t *info;

    totalsize = sizeof(chnid_t);
    for (i = 0; i < size; i++){
        totalsize += sizeof(msg_listdesc_t) + strlen(list[i].desc);     // 统计节目单信息有效数据的总长度
    }

    info = malloc(totalsize + sizeof(int)); // 1节目单1 这样的 所以最前面得加个int
    if (info == NULL){
        syslog(LOG_ERR, "malloc() : %s", strerror(errno));
        return -1;
    }
    memset(info, 0, totalsize + sizeof(int));
    info->len = totalsize;              // 将有效数据的总长度填入申请的内存中
    msg_list = info->msg;

    msg_list->chnid = LISTCHNID;        // 填入 LISTCHNID 频道号 0 号只广播频道信息
    desc_list = msg_list->list;


    for (i = 0; i < size; i++)          // 将节目单信息的有效数据内容填入申请的内存中
    {
        len = sizeof(msg_listdesc_t) + strlen(list[i].desc);
        desc_list->chnid = list[i].chnid;
        desc_list->deslength = htons(len);
        strncpy((void *)desc_list->desc, list[i].desc, strlen(list[i].desc));
        desc_list = (void *)(((char *)desc_list) + len);
    }
    threadpool_addtask(pool, sendlist, info); // 向任务队列添加一个任务
    return 0;
}
```

### channel

这个就是某个频道，一直发送数据了，不管有没有客户端都要一直发送

sendchannel里面调用了mlib_readchn，发送完一个mp3文件之后会列表循环，不会停止发送哈

之前也说了mlib_readchn里面使用了令牌桶进行了流量控制

```cpp
#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include <protocol.h>

int thr_channel_create(chnid_t);    // 流媒体音乐频道任务创建

#endif // !__CHANNEL_H__
```

```cpp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "channel.h"
#include "server_conf.h"
#include "medialib.h"

/*
 * @name            : sendchannel
 * @description		: 读取媒体库中流媒体的内容并发送
 * @param - arg     : msg_channel_t类型
 * @param - shut    : 线程池当前开启状态
 * @return 			: 无
 */
static void sendchannel(void *arg, volatile int *shut){
    msg_channel_t *context = (msg_channel_t *)arg;
    int len;
    while (*shut == 0){
        memset(context->data, 0, MAX_CHANNEL_DATA - sizeof(chnid_t));
        //读取频道里面的内容
        len = mlib_readchn(context->chnid, context->data, MAX_CHANNEL_DATA - sizeof(chnid_t));
        len = sendto(serversd, context, len + sizeof(chnid_t), 0, (void *)&sndaddr, sizeof(sndaddr));
        syslog(LOG_INFO, "%7s thread sendto %5d bytes, pool status is %d", "channel", len, *shut);
    }
}

/*
 * @name            : thr_channel_create
 * @description		: 流媒体音乐频道任务创建
 * @param - chnid   : 频道号
 * @return 			: 成功返回 0; 失败返回 -1
 */
int thr_channel_create(chnid_t chnid){
    msg_channel_t *context;
    context = malloc(MAX_CHANNEL_DATA);     // 申请内存，将本块内存地址传入线程池中
    if (context == NULL){
        syslog(LOG_ERR, "malloc() : %s", strerror(errno));
        // fprintf(stderr, "malloc() : %s\n", strerror(errno));
        return -1;
    }
    memset(context, 0, MAX_CHANNEL_DATA);
    context->chnid = chnid;
    threadpool_addtask(pool, sendchannel, context);     // context指向的内存由线程池负责释放
    return 0;
}
```

### server

前面都讲了，这里就没啥好讲的了，就是将所有的东西组合一下就好

两点注意：

1.守护进程

就是将整个的进程置于后台，终端就不输出了

就是fork一个子进程，然后脱离父进程，使用子进程来执行代码

将输出重定向到 /dev/null 里面，/dev/null是一个linux内置的一个设备，往里输出的话就表示，我不需要这些输出，直接丢弃

setsid 创建新会话，脱离终端，即使终端关闭了也不会断开

```cpp
static int daemon_init(){
    pid_t pid;
    int fd;
    pid = fork();
    if(pid < 0){
        syslog(LOG_ERR, "fork() : %s", strerror(errno));
        return -1;
    }else if(pid > 0){ //退出父进程
        //确保守护进程脱离父进程控制，成为后台进程。
        exit(EXIT_SUCCESS);
    }

    // dev/null 这是一个特殊的设备，输入进去之后，所有的东西都会被丢弃
    fd = open("/dev/null", O_RDWR);
    if(fd < 0){
        syslog(LOG_ERR, "open() : %s\n", strerror(errno));
        return -1;
    }
    // 守护进程的输入/输出/错误不再关联终端，避免干扰前台。
    dup2(fd, STDIN_FILENO);   // 标准输入
    dup2(fd, STDOUT_FILENO);  // 标准输出
    dup2(fd, STDERR_FILENO);  // 标准错误

    if (fd > STDERR_FILENO){
        close(fd);
    }

    chdir("/");    // 更改工作目录到根目录
    // 设置文件创建时的默认权限掩码。umask(0) 表示新文件权限为 0777 & ~0 = 0777（即 rwxrwxrwx）
    // 确保守护进程创建的文件/目录具有最大权限，由程序自行控制具体权限。
    umask(0);      // 重置文件权限掩码
    // 创建一个新的会话（Session），并成为会话的首进程。
    // 确保守护进程没有控制终端（TTY），即使终端关闭，守护进程仍可运行。
    setsid();      // 创建新会话，脱离终端控制

    return 0;
}
```

后面就没啥了，就是socket的创建以及各个函数的调用，这个看代码一看就明白

```cpp
#include <protocol.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "server_conf.h"
#include "threadpool.h"
#include "medialib.h"
#include "tokenbucket.h"
#include "channel.h"

#include "list.h"

int serversd;                 //全局性变量，在server_conf里面有声明 服务端套接字
ThreadPool_t *pool;           //全局性变量，在server_conf里面有声明 线程池
struct sockaddr_in sndaddr;   //全局性变量，在server_conf里面有声明 发送目的地址
static mlib_listdesc_t *list; //节目单

//全局性变量，在server_conf里面有声明 服务端的一些配置
server_conf_t server_conf =
    {
        .media_dir  = DEFAULT_MEDIADIR,
        .rcvport    = DEFAULT_RECVPORT,
        .runmode    = RUN_FOREGROUND,
        .ifname     = DEFAULT_IF,
        .mgroup     = DEFAULT_MGROUP
    };

// 命令行参数
struct option opt[] =
    {
        {"mgroup"  , required_argument, NULL, 'M'},
        {"port"    , required_argument, NULL, 'P'},
        {"mediadir", required_argument, NULL, 'D'},
        {"runmode" , required_argument, NULL, 'R'},
        {"ifname"  , required_argument, NULL, 'I'},
        {"help"    , no_argument      , NULL, 'H'}
    };

// 命令行参数帮助
static void print_help()
{
    printf("-M --mgroup     自定义多播组地址\n");
    printf("-P --port       自定义发送端口  \n");
    printf("-D --mediadir   自定义媒体库路径\n");
    printf("-R --runmode    自定义运行模式  \n");
    printf("-I --ifname     自定义网卡名称  \n");
    printf("-H --help       显示帮助       \n");
}

// 守护进程
static int daemon_init(){
    pid_t pid;
    int fd;
    pid = fork();
    if(pid < 0){
        syslog(LOG_ERR, "fork() : %s", strerror(errno));
        return -1;
    }else if(pid > 0){ //退出父进程
        //确保守护进程脱离父进程控制，成为后台进程。
        exit(EXIT_SUCCESS);
    }

    // dev/null 这是一个特殊的设备，输入进去之后，所有的东西都会被丢弃
    fd = open("/dev/null", O_RDWR);
    if(fd < 0){
        syslog(LOG_ERR, "open() : %s\n", strerror(errno));
        return -1;
    }
    // 守护进程的输入/输出/错误不再关联终端，避免干扰前台。
    dup2(fd, STDIN_FILENO);   // 标准输入
    dup2(fd, STDOUT_FILENO);  // 标准输出
    dup2(fd, STDERR_FILENO);  // 标准错误

    if (fd > STDERR_FILENO){
        close(fd);
    }

    chdir("/");    // 更改工作目录到根目录
    // 设置文件创建时的默认权限掩码。umask(0) 表示新文件权限为 0777 & ~0 = 0777（即 rwxrwxrwx）
    // 确保守护进程创建的文件/目录具有最大权限，由程序自行控制具体权限。
    umask(0);      // 重置文件权限掩码
    // 创建一个新的会话（Session），并成为会话的首进程。
    // 确保守护进程没有控制终端（TTY），即使终端关闭，守护进程仍可运行。
    setsid();      // 创建新会话，脱离终端控制

    return 0;
}

/* 信号处理函数：清理资源并退出 */
static void daemon_exit(int s) {
    threadpool_destroy(pool);     // 销毁线程池
    mlib_freechnlist(list);       // 释放频道列表内存
    mlib_freechncontext();        // 释放频道上下文资源
    tokenbt_shutdown();           // 关闭令牌桶
    close(serversd);              // 关闭套接字
    closelog();                   // 关闭系统日志
    exit(EXIT_SUCCESS);           // 正常退出
}

/* 初始化UDP套接字 */
static int socket_init() {
    int ret;
    struct ip_mreqn mreq; // 多播请求结构体
    /*struct ip_mreqn：用于配置多播组的参数，包含以下字段：
        imr_multiaddr：多播组 IP 地址。
        imr_address：本地接口 IP 地址。
        imr_ifindex：网卡接口的索引。 
    */
    // 创建UDP套接字
    serversd = socket(AF_INET, SOCK_DGRAM, 0);
    if (serversd < 0) {
        syslog(LOG_ERR, "socket() : %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // 设置多播组参数
    inet_pton(AF_INET, server_conf.mgroup, &mreq.imr_multiaddr); // 多播组地址
    inet_pton(AF_INET, "0.0.0.0", &mreq.imr_address);           // 本地任意地址
    mreq.imr_ifindex = if_nametoindex(server_conf.ifname);     // 网卡名称转索引
    /*
    inet_pton：将字符串形式的 IP 地址转换为二进制格式。
    server_conf.mgroup：配置中的多播组地址（如 "239.0.0.1"）。
    "0.0.0.0"：表示绑定到本地所有可用接口。
    if_nametoindex：将网卡名称（如 "eth0"）转换为系统索引。
    若网卡不存在，此函数会失败，但代码中未处理该错误。
     */

     // 告诉内核通过指定的网卡（server_conf.ifname）发送多播数据。
    ret = setsockopt(serversd, IPPROTO_IP, IP_MULTICAST_IF, &mreq, sizeof(mreq));
    if (ret < 0)
    {
        syslog(LOG_ERR, "setsockopt() : %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    sndaddr.sin_family = AF_INET;
    sndaddr.sin_port = htons(atoi(server_conf.rcvport));
    inet_pton(AF_INET, server_conf.mgroup, &sndaddr.sin_addr);

    return 0;
}

int main(int argc, char **argv) {
    int i, ret;
    int arg;
    int list_size;                // 频道列表大小
    struct sigaction action;      // 信号处理结构体

    // 打开系统日志，标识为"netradio"，包含PID和输出到stderr
    openlog("netradio", LOG_PID | LOG_PERROR, LOG_DAEMON);

    while (1) {
        arg = getopt_long(argc, argv, "M:P:D:R:I:H", opt, NULL);
        if (arg == -1) //没有参数就都是默认的，不用解析
            break;
        switch (arg) {
        case 'M':
            server_conf.mgroup = optarg; // 设置多播组地址
            break;
        case 'P':
            server_conf.rcvport = optarg; // 设置接收端口
            break;
        case 'D':
            server_conf.media_dir = optarg; // 设置媒体目录
            break;
        case 'R':
            // 设置运行模式（0:前台，1:守护进程）
            if (atoi(optarg) == 1 || atoi(optarg) == 0) {
                server_conf.runmode = (enum RNUMODE)atoi(optarg);
            } else {
                syslog(LOG_ERR, "参数错误！详见");
                print_help();
                exit(EXIT_FAILURE);
            }
            break;
        case 'I':
            server_conf.ifname = optarg; // 设置网卡名称
            break;
        case 'H':
            print_help(); // 显示帮助
            exit(EXIT_SUCCESS);
        default:
            syslog(LOG_ERR, "参数错误！详见");
            print_help();
            exit(EXIT_FAILURE);
        }
    }
    syslog(LOG_INFO, "当前配置：\n多播组IP：\t%s\n端口：\t\t%s\n媒体库路径：\t%s\n运行模式：\t%d\n网卡名：\t%s\n",
        server_conf.mgroup,
        server_conf.rcvport,
        server_conf.media_dir,
        server_conf.runmode,
        server_conf.ifname);
    // 判断运行模式，如果是守护进程模式则初始化
    if (server_conf.runmode == RUN_DAEMON)  // 判断系统运行模式
    {
        ret = daemon_init();
        if (ret < 0){
            syslog(LOG_ERR, "daemon_init() failed ...");
            exit(EXIT_FAILURE);
        }
    }

    // 信号处理
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);    // 清空信号掩码
    sigaddset(&action.sa_mask, SIGINT);  // Ctrl+C
    sigaddset(&action.sa_mask, SIGQUIT); // Ctrl+\ //
    sigaddset(&action.sa_mask, SIGTSTP); // Ctrl+Z
    action.sa_handler = daemon_exit;      // 绑定处理函数 //都是退出
    sigaction(SIGINT, &action, NULL); // 注册信号捕捉函数
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGTSTP, &action, NULL);

    // 初始化套接字
    socket_init();

    // 创建线程池（核心5线程，最大20线程，队列容量20）
    pool = threadpool_create(5, 20, 20);
    if (pool == NULL) {
        syslog(LOG_ERR, "threadpool_create() : failed ...");
        exit(EXIT_FAILURE);
    }

    // 获取媒体库频道列表
    ret = mlib_getchnlist(&list, &list_size);
    if (ret < 0) {
        syslog(LOG_ERR, "mlib_getchnlist() : failed ...");
        exit(EXIT_FAILURE);
    }

    // 创建频道列表线程
    ret = thr_list_create(list, list_size);
    if (ret < 0) {
        syslog(LOG_ERR, "thr_list_create() : failed ...");
        exit(EXIT_FAILURE);
    }

    // 为每个频道创建发送线程
    for (i = 0; i < list_size; i++) {
        ret = thr_channel_create(list[i].chnid);
        if (ret < 0) {
            syslog(LOG_ERR, "thr_channel_create() : failed ...");
            exit(EXIT_FAILURE);
        }
    }

    // 主循环挂起，等待信号
    while (1)
        pause();  // 阻塞主线程，等待信号 这里要注意哈，为舍么要阻塞？因为ie前面发送节目单信息以及每个频道里面的信息都申请了 额外的线程来做了，这里就没必要再做了

    exit(EXIT_SUCCESS);
}
```


### 客户端

任务就是接收数据，两个进程，父进程进行数据的接收，输入到管道里面，然后子进程拿数据输入到ffmpeg里面来播放，就是这个流程, 基本没有啥坑

唯一需要注意的点就是如何选择自己的频道

首先要明白一点，加入到组播里面之后，client会接收所有的数据

因此需要先进行判断，首先得先拿到节目单，也就是ID为0的包。不是ID为0的包就丢弃。

然后把包里面的内容输出到终端让客户来选择那个频道，比如输入1，那么就选择1频道

后面拿到包之后先对比一下是不是1频道，是的话就拿过来，处理。不是的话就丢弃就好。

其他的就没啥的，下面是代码


```cpp
#ifndef __CLIENT_H__
#define __CLIENT_H__

// #define DEFAULT_PALYERCMD "/usr/bin/mplayer -"     使用mpg123播放器并将输出重定向到/dev/null（静默模式）
// /dev/null 是一个特殊的文件，通常被称为“空设备”或“位桶”（bit bucket） 
// 任何写入 /dev/null 的内容都会被永久丢弃，不会保存在任何地方。同时，从 /dev/null 读取内容时，总是会返回空（EOF，即文件结束符）。
#define DEFAULT_PALYERCMD "/usr/bin/mpg123 - > /dev/null" //mpg123播放mp3文件，并且把一些输出给丢弃掉 

#include <stdint.h>

typedef struct client_conf_t
{
    char *mgroup;       // 多播组IP地址
    char *revport;      // 接收端口
    char *playercmd;    // 播放器命令
} client_conf_t;

#endif // !__CLIENT_H__
```


```cpp
#include <protocol.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <net/if.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include "client.h"

// 定义全局变量，程序退出时释放内存。 
msg_list_t *msg_list = NULL;        // 节目单数据缓冲区指针
msg_channel_t *msg_channel = NULL;  // 频道数据缓冲区指针
int sfd;                            // 套接字文件描述符

client_conf_t conf = // client 配置
    {
        .mgroup = DEFAULT_MGROUP,         // 默认多播组地址
        .revport = DEFAULT_RECVPORT,      // 默认接收端口
        .playercmd = DEFAULT_PALYERCMD};  // 默认播放器命令

// 命令行参数解析
struct option opt[] =
    {
        {"mgroup", required_argument, NULL, 'M'},       // 多播组地址选项
        {"port", required_argument, NULL, 'P'},         // 接收端口选项
        {"player", required_argument, NULL, 'p'},       // 播放器命令选项
        {"help", no_argument, NULL, 'H'}};              // 帮助选项

// 命令行参数帮助 // 打印帮助信息
static void print_help()
{
    printf("-M --mgroup 自定义多播组地址\n");
    printf("-P --port   自定义接收端口  \n");
    printf("-p --player 自定义音乐解码器\n");
    printf("-H --help   显示帮助       \n");
}

/*
 * @name            : writen
 * @description		: 自定义封装函数，保证写足 count 字节
 * @param - fd 	    : 文件描述符
 * @param - buf 	: 要写入的内容
 * @param - count 	: 要写入的内容总长度
 * @return 			: 成功返回写入的字节数; 失败返回 -1
 */
static ssize_t writen(int fd, const void *buf, size_t count){
    size_t len, total, ret;
    total = count;
    for (len = 0; total > 0; len += ret, total -= ret){
    again:
        ret = write(fd, buf + len, total);
        if (ret < 0)
        {
            if (errno == EINTR) // 中断系统调用，重启 write
                goto again;
            fprintf(stderr, "write() : %s\n", strerror(errno));
            return -1;
        }
    }
    return len;

}

/*
 * @name            : exit_action
 * @description		: 信号捕捉函数，用于退出前清理
 * @param - s 	    : 信号
 * @return 			: 无
 */
static void exit_action(int s){
    pid_t pid;
    pid = getpgid(getpid());  // 获取进程组ID
    // 释放动态分配的内存
    if (msg_list != NULL)
        free(msg_list);
    if (msg_channel != NULL)
        free(msg_channel);
    close(sfd);           // 关闭套接字
    kill(-pid, SIGQUIT);  // 向进程组发送退出信号
    fprintf(stdout, "\nthis programme is going to exit...\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv){

    int arg;       // 临时存储getopt返回值
    int ret;       // 通用返回值存储
    int len;       // 接收数据长度
    int val;       // 临时整数值存储
    int chosen;    // 用户选择的频道ID
    int fd[2];     // 管道文件描述符数组
    char ip[20];   // 存储IP字符串
    pid_t pid;     // 进程ID
    socklen_t socklen;   // 套接字地址结构长度
    uint64_t receive_buf_size = 20 * 1024 * 1024; // 20MB  // 接收缓冲区大小（20MB）
    struct ip_mreqn group;      // 多播组结构体
    struct sockaddr_in addr,    // 本地绑定地址
                    list_addr,  // 节目单来源地址
                    data_addr;  // 数据来源地址
    struct sigaction action;    // 信号处理结构体

    while (1)
    {
        arg = getopt_long(argc, argv, "P:M:p:H", opt, NULL); // 命令行参数解析
        if (arg == -1)
            break;
        switch (arg)
        {
        case 'P':
            conf.revport = optarg;      // 设置接收端口
            break;
        case 'M':
            conf.mgroup = optarg;       // 设置多播组地址
            break;
        case 'p':
            conf.playercmd = optarg;    // 设置播放器命令
            break;
        case 'H':                       // 显示帮助
            print_help();
            break;
        default:
            fprintf(stderr, "参数错误！详见\n");
            print_help();
            exit(EXIT_FAILURE);
            break;
        }
    }
    // 打印当前配置
    fprintf(stdout, "当前配置：\n多播组IP:\t%s\n端口：\t\t%s\n播放器：\t%s\n",
            conf.mgroup, conf.revport, conf.playercmd);

    ret = pipe(fd); // 创建匿名管道创建管道用于与播放器通信
    if (ret < 0){
        fprintf(stderr, "pipe() : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    pid = fork(); // 创建子进程 // 创建子进程用于运行播放器
    if (pid < 0){  //创建失败
        fprintf(stderr, "fork() : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }else if(pid == 0){  //子进程负责将父进程读取到的数据通过管道馈入到ffmpeg里面播放
        close(fd[1]);                 // 关闭写端（子进程只读）
        dup2(fd[0], STDIN_FILENO);    // 将管道读端重定向到标准输入
        close(fd[0]);                 // 关闭原始读端
        execl("/bin/sh", "sh", "-c", conf.playercmd, NULL); // 使用shell解释器来运行 mpg123，子进程被替换成mpg123
        fprintf(stderr, "execl() : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    // 父进程
    close(fd[0]); //父进程不需要读管道
    // 配置信号处理结构体
    action.sa_flags = 0;                   // 不使用特殊标志
    sigemptyset(&action.sa_mask);          // 清空信号屏蔽字
    // 添加要阻塞的信号（处理期间屏蔽这些信号）
    sigaddset(&action.sa_mask, SIGINT);    // Ctrl+C
    sigaddset(&action.sa_mask, SIGQUIT);   // Ctrl+\ //
    sigaddset(&action.sa_mask, SIGTSTP);   // Ctrl+Z
    action.sa_handler = exit_action;       // 设置统一信号处理函数
    // 注册信号处理器
    sigaction(SIGINT, &action, NULL);   // 注册Ctrl+C处理
    sigaction(SIGQUIT, &action, NULL);  // 注册退出处理
    sigaction(SIGTSTP, &action, NULL);  // 注册暂停处理

    // 创建UDP套接字
    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    // 设置绑定地址结构
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);
    addr.sin_port = htons(atoi(conf.revport));
    // 绑定套接字
    ret = bind(sfd, (void *)&addr, sizeof(addr)); // 绑定本地 IP ，端口

    if (ret < 0){
        fprintf(stderr, "bind() : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // 设置接收缓冲区大小
    ret = setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, &receive_buf_size, sizeof(receive_buf_size)); // 设置套接字接收缓冲区 20 MB
    if (ret < 0)
    {
        fprintf(stderr, "SO_RCVBUF : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // 允许组播数据回环（本机接收自己发送的数据）
    val = 1;
    ret = setsockopt(sfd, IPPROTO_IP, IP_MULTICAST_LOOP, &(val), sizeof(val)); // 允许组播数据包本地回环
    if (ret < 0)
    {
        fprintf(stderr, "IP_MULTICAST_LOOP : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

        // 设置多播组参数
    inet_pton(AF_INET, conf.mgroup, &group.imr_multiaddr);       // 多播组地址
    inet_pton(AF_INET, "0.0.0.0", &group.imr_address);           // 本地接口地址
    group.imr_ifindex = if_nametoindex("ens33");                                 // 绑定自己的网卡
    ret = setsockopt(sfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group)); // 加入多播组

    if (ret < 0)
    {
        fprintf(stderr, "IP_ADD_MEMBERSHIP() : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    // 分配节目单数据缓冲区
    msg_list = malloc(MAX_LISTCHN_DATA);
    if (msg_list == NULL){ //错误处理
        fprintf(stderr, "malloc() : %s\n", strerror(errno));
    }
    socklen = sizeof(struct sockaddr_in);

    while(1){
        memset(msg_list, 0, MAX_LISTCHN_DATA);    // 清空缓冲区
        len = recvfrom(sfd, msg_list, MAX_LISTCHN_DATA, 0, (void *)&list_addr, &socklen); // 接收节目单包
        if (len < sizeof(msg_list_t))
        {
            fprintf(stderr, "data is too short, len = %d...\n", len);
            continue;
        }
         // 检查是否为节目单数据（根据预设的LISTCHNID判断）
        if (msg_list->chnid == LISTCHNID) // 如果是节目单包则保留，不是则丢弃
        {
            fprintf(stdout, "list from IP = %s, Port = %d\n",
                    inet_ntop(AF_INET, &list_addr.sin_addr, ip, sizeof(ip)),
                    ntohs(list_addr.sin_port));
            break;
        }
    }

    // 解析并打印节目单信息
    msg_listdesc_t *desc;
    for (desc = msg_list->list; (char *)desc < (char *)msg_list + len; desc = (void *)((char *)desc + ntohs(desc->deslength)))
    {
        fprintf(stdout, "chnid = %d, description = %s\n", desc->chnid, desc->desc);
    }
    free(msg_list); // 释放节目单缓冲区
    msg_list = NULL;

    // 用户选择频道
    fprintf(stdout, "请输入收听的频道号码，按回车结束！\n");
    while (1)
    {
        fflush(NULL);
        ret = scanf("%d", &chosen);
        if (ret != 1)
            exit(EXIT_FAILURE);
        else if (ret == 1)
            break;
    }

    // 分配频道数据缓冲区
    msg_channel = malloc(MAX_CHANNEL_DATA);
    if (msg_channel == NULL)
    {
        fprintf(stderr, "malloc() : %s\n", strerror(errno));
    }

    // 持续接收数据
    socklen = sizeof(struct sockaddr_in);

    while(1){
        memset(msg_channel, 0, MAX_CHANNEL_DATA);     // 清空缓冲区
        len = recvfrom(sfd, msg_channel, MAX_CHANNEL_DATA, 0, (void *)&data_addr, &socklen); // 接收频道内容包
        if (len < sizeof(msg_channel_t)){ // UDP不是面向字节流的，接收的话就是一个包
            fprintf(stderr, "data is too short, len = %d...\n", len);
            continue;
        }else if(data_addr.sin_addr.s_addr != list_addr.sin_addr.s_addr || data_addr.sin_port != list_addr.sin_port){
        // 验证数据包和节目单数据包是否为同一服务端发送，防止干扰
            fprintf(stderr, "data is not match!\n");
            continue;
        }

        if (msg_channel->chnid == chosen)
        {
            fprintf(stdout, "recv %d length data!\n", len);
            ret = writen(fd[1], msg_channel->data, len - sizeof(msg_channel->chnid)); // 写入管道
            if (ret < 0){
                exit(EXIT_FAILURE);
            }
        }
    }
    exit(EXIT_SUCCESS);
}
```

# 总结

感觉比webserver还要简单一些，就是这个里面的结构体各种定义很容易迷，这就是不如C++ class封装之处了。整体来看还是很值的做的。