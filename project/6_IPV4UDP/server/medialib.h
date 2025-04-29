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