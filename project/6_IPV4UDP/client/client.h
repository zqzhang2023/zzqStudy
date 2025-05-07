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