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
}

#endif