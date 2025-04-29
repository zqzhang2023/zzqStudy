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