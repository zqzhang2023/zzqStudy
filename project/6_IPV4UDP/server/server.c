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