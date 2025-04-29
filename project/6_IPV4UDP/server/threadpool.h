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