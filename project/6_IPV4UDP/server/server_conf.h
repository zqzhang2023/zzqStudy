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