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