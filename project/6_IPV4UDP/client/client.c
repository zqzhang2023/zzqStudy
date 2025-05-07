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