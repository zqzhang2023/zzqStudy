# chatRoom 项目

git:https://github.com/zqzhang2023/zzqStudy

简单介绍：socket的简单应用，简单来说，就是一个服务端：server 许多个客户端：client1 client2 client3 client4... 现在要把这么多客户端拉到一个聊天室里面，实现群聊通信

实现方式：其实思想很简单，就是 client_i 向 server发送消息，然后server接收到消息之后，把消息转发给除却 client_i 之外的所有的 客户

这里主要学习 I/O多路复用技术 之前学习的基础的 socket 是客户端 服务器 一对一的，现在需要服务器接收多个客户端的连接，因此需要用到 I/O多路复用技术

# I/O多路复用技术

## 为什么使用

socket 中的许多操作会阻塞进程，比如服务端的accept 还有两端的 recv 等操作，如果要接受多个客户端，就不能阻塞进程。

有一种方法就是 多线程操作，每个线程接收一个，但是这种方法会严重的浪费内存空间，高并发的程序中 上千个客户端，就需要申请上千个线程

而且每一次切换线程 可能会需要 进行上下文操作，这就会很大的限制运行速度


## select 技术

这是比较早的技术

```cpp
//select函数接口
#include <sys/select.h>
#include <sys/time.h>
 
#define FD_SETSIZE 1024
#define NFDBITS (8 * sizeof(unsigned long))
#define __FDSET_LONGS (FD_SETSIZE/NFDBITS)
 
// 数据结构 (bitmap)
typedef struct {
    unsigned long fds_bits[__FDSET_LONGS];
} fd_set;
 
// API
int select(
    int max_fd, 
    fd_set *readset, 
    fd_set *writeset, 
    fd_set *exceptset, 
    struct timeval *timeout
)                              // 返回值就绪描述符的数目
 
FD_ZERO(int fd, fd_set* fds)   // 清空集合
FD_SET(int fd, fd_set* fds)    // 将给定的描述符加入集合
FD_ISSET(int fd, fd_set* fds)  // 判断指定描述符是否在集合中 
FD_CLR(int fd, fd_set* fds)    // 将给定的描述符从文件中删除  
```

主要再于 select API 

说一下怎么工作的：我们注意到select之中会有三个 fd_set ，第一个是读事件，第二个是写事件，第三个是异常事件 这三个是咱们需要监听的事件

### 举例说明：

- 比如咱们有 5 个 文件描述符需要监控 3 5 7 8 10，咱们需要告诉内核态，咱们需要监控这些 文件描述符 的那些事件
- 对于 readset 咱们传入 (注意下标)0 0 0 1 0 1 0 1 1 0 0 (咱们将这个bitmap命名为readsetName哈)  表示 对于 3 5 7 8 咱们要监控读事件
  - 注意哈，这里是举个例子，并不是真实的，因为有些位置是操作系统专用的(我记得有些资料上说前三个是系统专用的)，咱们不能使用，这只是便于理解，其实不是这样的
- 同理，对于 writeset 咱们传入 0 0 0 1 0 1 0 0 0 0 1 (咱们将这个bitmap命名为writesetName哈)表示 对于 3 5 10 咱们要监控写事件  exceptset 同理
- 这个时候会将 **全部的文件描述符都放入内核态** 内核态会一直 **遍历轮询查询**，直到咱们监控的事件发生，其会修改咱们传入的readsetName等，这里以readsetName为例
  - 首先会将readsetName都重置为0
  - 比如 3 5 这两个事件发生了 read事件(有字符传进来了) 那么内核态度就会修改readsetName对应位置为1,就是：
  - readsetName = 0 0 0 1 0 1 0 0 0 0 0 表示 3 5 两个事件发生了read事件
  - select返回值是发生事件文件描述符的数量
- 后面咱们通过对readsetName进行遍历，就能知道那那一个 文件描述符 发生事件，然后做对应的处理就好

### select的缺点：
- select 使用固定长度的 BitsMap表示文件描述符集合，单个进程所打开的fd是有限制的，通过FD_SETSIZE设置，默认1024
- 每次调用select，都要把fd集合从用户态拷贝到内核态，在多个fd时开销较大
- 对socket扫描时是线性扫描，采用轮询的方式，效率较低（高并发时）

## poll 技术

```cpp

#include <poll.h>
// 数据结构
struct pollfd {
    int fd;                         // 需要监视的文件描述符
    short events;                   // 需要内核监视的事件
    short revents;                  // 实际发生的事件
};
 
// API
int poll(struct pollfd fds[], nfds_t nfds, int timeout);
```

引入了pollfd结构体，不需要咱们再定义三个BitsMap来监控了，但是其实还是和select 技术差不多

### 举例说明

- 还是比如咱们有 5 个 文件描述符需要监控 3 5 7 8 10，这个时候咱们有 5 个 上面所说的结构体
- 咱们通过定义 events 来定义咱们需要监控的事件
- 还是 **全部的文件描述符都放入内核态**  内核态还是会一直 **遍历轮询查询** 当有事件发生的时候，修改 对应结构体的 revents 然后返回 
- 咱们在用户态一个一个查询对应结构体的revents，就能知道是那个文件描述符发生了啥事件
- 后面咱们再做对应处理
  - 这里要注意，咱们做处理之后要把对应的revents置0,这里和select还是有些差别的select会自动置0
- 这个其实和 select 技术差不多，相当于 select 的一个小优化

会使用 一个数组存储(其实是链表) pollfd 结构体，因此不会存在1024的限制

### poll的缺点
- 每次调用poll，都需要把fd集合从用户态拷贝到内核态，多个fd时开销很大
- 对socket扫描时是线性扫描，采用轮询的方式，效率较低（高并发时）

## epoll

```cpp
#include <sys/epoll.h>
 
// 数据结构
// 每一个epoll对象都有一个独立的eventpoll结构体
// 用于存放通过epoll_ctl方法向epoll对象中添加进来的事件
// epoll_wait检查是否有事件发生时，只需要检查eventpoll对象中的rdlist双链表中是否有epitem元素即可
struct eventpoll {
    /*红黑树的根节点，这颗树中存储着所有添加到epoll中的需要监控的事件*/
    struct rb_root  rbr;
    /*双链表中则存放着将要通过epoll_wait返回给用户的满足条件的事件*/
    struct list_head rdlist;
};
 
// API
int epoll_create(int size); // 内核中间加一个 ep 对象，把所有需要监听的 socket 都放到 ep 对象中
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event); // epoll_ctl 负责把 socket 增加、删除到内核红黑树
int epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout);// epoll_wait 负责检测可读队列，没有可读 socket 则阻塞进程

```

### 介绍以及原理简单理解

注意这里介绍一下基础的功能(我查询资料的时候eventpoll还有一些其他的像阻塞进程队列的功能，有些有有些没有，我拿最为基础的功能来理解了)和自己的理解，可能有些不对的地方

以上两种方式没有解决需要多次在用户态和内核态切换造成大量数据开销和轮询扫描socket导致效率低的问题 epoll 就解决了这个问题

#### 基本原理：
- eventpoll在创建好之后就会直接放进内核态，因此不需要像select 和 poll那样反复的
- 首先eventpoll会维护两个“存储的桶” 一个用来存储**需要监控的文件描述符** 一个用来存储**发生事件**的文件描述符
- eventpoll会为 需要监控的文件描述符 维护一个独特的 回调机制，这里不再需要轮询遍历查询，当某个 文件描述符 的事件发生时，内核会通过回调函数，将这个 文件描述符 放入 存储 发生事件 的 桶 里面 (这个回调机制我也没搞懂)
- 后面 epoll_wait 只需要 将 发生事件 的 桶 里面的事件返回就好

#### 结构
- 对于存储 需要监控的文件描述符 的 桶
  - 要方便的添加和移除，还要便于搜索还要避免重复添加 因此 选择 红黑树
- 对于存储 发生事件的文件描述符 的 桶
  - 可能随时添加可能随时删除，因此就绪列表应是一种能够快速插入和删除的数据结构，epoll选择了双向链表来实现就绪队列
  
### 举个例子
- 还是比如咱们有 5 个 文件描述符需要监控 3 5 7 8 10
- 先创建一个eventpoll(epoll_create)，这个时候eventpoll已经在内核态里面了
- 咱们通过epoll_ctl将这5个文件描述符逐一ADD进eventpoll之中，eventpoll会使用一个红黑树来存储（rbr）
- 调用epoll_wait来监听 (epoll_wait会阻塞进程哈)
- 当有某个文件描述符发生了对应的监听的事件之后，eventpoll会将对应的文件描述符添加到发生事件的文件描述符 的 桶 也就是 队列 rdlist 之中
- epoll_wait将对应文件描述符号写入 events(这是个数组，是epoll_wait的参数) 之中，返回的是发生事件的个数
- 因此，咱们可以直接访问events数组来处理对应的事件 并且 因为返回了发生事件的个数，也不怕数组越界

### 一些API

- 创建epoll函数
```cpp
#include <sys / epoll.h>
int epoll_create(int size)

size：最大监听的fd+1
return：成功返回文件描述符fd；失败返回-1，可根据错误码判断错误类型
```

- epoll事件的注册函数
```cpp
int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event)

epfd：epoll_create()返回的epoll fd
op：操作值
fd：需要监听的fd
event：需要监听的事件
return：成功返回0；失败返回-1，可根据错误码判断错误类型
```
- epoll事件的注册函数
  - epoll_ctl()中操作数op的三种操作类型
    - EPOLL_CTL_ADD：注册目标fd到epoll fd中，同时关联event到fd上
    - EPOLL_CTL_MOD：修改已经注册到fd的监听事件
    - EPOLL_CTL_DEL：从epoll fd中删除/移除已注册的fd
  - epoll_ctl()中事件event的枚举如下:
    - EPOLLIN：表示关联的fd可以进行读操作
    - EPOLLOUT：表示关联的fd可以进行写操作
    - EPOLLRDHUP：表示socket关闭了连接（Linux2.6.17后上层只需通过EPOLLRDHUP判断对端是否关闭socket，减少一次系统调用）
    - EPOLLPRI：表示关联的fd有紧急优先事件可以进行读操作
    - EPOLLERR：表示关联的fd发生了错误，epoll_wait会一直等待这个事件，一般无需设置该属性
    - EPOLLHUP：表示关联的fd挂起，epoll_wait会一直等待这个事件，一般无需设置该属性
    - EPOLLET：设置关联的fd为ET的工作方式，epoll默认的工作方式是LT
    - EPOLLONESHOT：设置关联的fd为one-shot的工作方式，表示只监听一次事件，如果要再次监听，则需再次把该socket放入epoll队列中

当socket接收到数据后，中断程序会给eventpoll的就绪列表“rdlist”添加socket引用，而不是直接唤醒进程

- epoll等待事件函数

```cpp
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)

epfd：epoll描述符
events：分配好的 epoll_event结构体数组，epoll将会把发生的事件复制到 events数组中（events不可以是空指针，内核只负责把数据复制到这个 events数组中，不会去帮助我们在用户态中分配内存。内核这种做法效率很高）
maxevents：本次可以返回的最大事件数目，通常与预分配的events数组的大小是相等的
timeout：在没有检测到事件发生时最多等待的时间（单位为毫秒），如果 timeout为0，则表示 epoll_wait在 rdllist链表中为空，立刻返回，不会等待
return：成功返回需要处理的事件数目，返回0表示已超时；失败则返回-1，可以根据错误码判断错误类型
```

### epoll的常用框架

```cpp
for( ; ; )
    {
        nfds = epoll_wait(epfd,events,20,500);
        for(i=0;i<nfds;++i)
        {
            if(events[i].data.fd==listenfd) //有新的连接
            {
                connfd = accept(listenfd,(sockaddr *)&clientaddr, &clilen); //accept这个连接
                ev.data.fd=connfd;
                ev.events=EPOLLIN|EPOLLET;
                epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev); //将新的fd添加到epoll的监听队列中
            }
            else if( events[i].events&EPOLLIN ) //接收到数据，读socket
            {
                n = read(sockfd, line, MAXLINE)) < 0    //读
                ev.data.ptr = md;     //md为自定义类型，添加数据
                ev.events=EPOLLOUT|EPOLLET;
                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);//修改标识符，等待下一个循环时发送数据，异步处理的精髓
            }
            else if(events[i].events&EPOLLOUT) //有数据待发送，写socket
            {
                struct myepoll_data* md = (myepoll_data*)events[i].data.ptr;    //取数据
                sockfd = md->fd;
                send( sockfd, md->ptr, strlen((char*)md->ptr), 0 );        //发送数据
                ev.data.fd=sockfd;
                ev.events=EPOLLIN|EPOLLET;
                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev); //修改标识符，等待下一个循环时接收数据
            }
            else
            {
                //其他的处理
            }
        }
    }

```


# chatRoom 项目

好了，现在你已经学会了 epoll 开始写项目吧

epoll了解了，并且了解了socket那么这个项目就显得简单了

下面是添加了注释的代码，这个看一遍就能看懂，不需要多讲了

（这里我只在本机上进行测试，其实是可以把server放在服务器上的,然后通过公网访问）

```cpp
// server.cpp
#include<iostream>
#include<string>
#include<sys/epoll.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<map>
using namespace std;

//最大链接数
const int MAX_CONN = 1024;

//保存客户端的信息
struct Client {
    int sockfd;  //客户端 socket 文件间描述符
    string name; //客户端 name
};

int main(){

    //服务端 socket
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0){
        perror("socket error");
        return -1;
    }

    //服务器地址信息
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9999);

    //绑定一下
    if(bind(sockfd,(struct sockaddr*)&addr,sizeof(addr)) < 0){
        perror("bind error");
        return -1;
    }

    //监听端口
    if(listen(sockfd,MAX_CONN) < 0){
        perror("listen error");
        return -1;
    }

    //创建epoll实例
    //注意这里哈，本来应该使用epoll_create(size)来创建的 但是 咱们事先也不知道size是多少，这种情况很常见
    //因此epoll推出了epoll_create1的api，不需要再传size了 0 是flag 咱也不知道为啥要有这个 就这样用吧
    int epid = epoll_create1(0);  
    if(epid < 0){
        perror("epoll create error");
        return -1;
    }

    //sockfd 绑定事件
    struct epoll_event ev;
    ev.events = EPOLLIN; //监听读事件
    ev.data.fd = sockfd;

    //把节点加入到 epoll 模型之中
    if(epoll_ctl(epid,EPOLL_CTL_ADD,sockfd,&ev) < 0){
        perror("epoll_ctl error");
        return -1;
    }

    //保存客户端信息
    map<int,Client> clients;

    //循环监听
    while(1){
        // 这个用来获取那些就绪的文件描述符
        struct epoll_event evs[MAX_CONN];
        // epoll_wait 最后一个参数 -1 表示，不需要设置超时时间限制
        int n = epoll_wait(epid,evs,MAX_CONN,-1);
        if(n < 0){
            perror("epoll_wait error");
            break;
        }
        // 处理那些有响应的事件
        for(int i=0;i<n;i++){
            int fd = evs[i].data.fd;
            //如果是服务器的fd受到消息，那么则表示有客户端进行链接了
            if(fd == sockfd){
                //那就 accept 一下
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int client_sockfd = accept(sockfd,(struct sockaddr*)&client_addr,&client_addr_len);
                if(client_sockfd < 0){
                    perror("accept error");
                    break;
                }

                //将客户端的sockfd加入epoll
                struct epoll_event ev_client;
                ev_client.events = EPOLLIN; //监听读事件 客户端有没有消息过来
                ev_client.data.fd = client_sockfd;
                //把节点加入到 epoll 模型之中
                if(epoll_ctl(epid,EPOLL_CTL_ADD,client_sockfd,&ev_client) < 0){
                    perror("epoll_ctl client error");
                    break;
                }
                cout<<client_addr.sin_addr.s_addr<<" 正在连接..."<<endl;
                // 保存客户端信息
                Client client;
                client.sockfd = client_sockfd;
                client.name = "";
                // 存储进map
                clients[client_sockfd] = client;

            }else{ // 不是服务器的sockfd 那就是客户端来消息了
                char buffer[1024];  //接收信息的缓冲区
                int recv_len = recv(fd,buffer,1024,0);
                if(recv_len<0){
                    perror("recv client error");
                    break;
                }else if(recv_len == 0){ //代表要断开连接了
                    close(fd);
                    epoll_ctl(epid,EPOLL_CTL_DEL,fd,0); // 把fd移除
                    clients.erase(fd); //map里面也移除一下
                }else{ //正常的发消息
                    string msg(buffer,recv_len);//先转成string
                    //客户端，还没有名字，先把名字设置了
                    if(clients[fd].name == ""){
                        clients[fd].name = msg;
                    }else{ //否则服务端就将信息转发到除发送端之外所有的客户端
                        string name = clients[fd].name;
                        for(auto &c:clients){
                            if(c.first != fd){
                                send(c.first,('['+name+']'+": "+msg).c_str(),msg.size()+name.size()+4,0);
                            }
                        }
                    }
                }

            }
        }
    }

    //关闭
    close(epid);
    close(sockfd);
    
    return 0;
}
```

```cpp
// client.cpp
#include<iostream>
#include<string>
#include<sys/epoll.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<thread>
#include<string.h>
using namespace std;

#define BUF_SIZE 1024        //缓冲区大小
char sendMsgBuff[BUF_SIZE];  //发送消息缓冲区
char recvMsgBuff[BUF_SIZE];  //接收消息缓冲区

bool flag = true;            //false表示结束

void sendMsg(int clientfd){
    //清空buf
    memset(sendMsgBuff, 0, BUF_SIZE);
    while(flag){
        cin>>sendMsgBuff;  //注意这里可能不能处理 空格 哈，可以使用其他的函数，但是这不是重点，就是用cin这个简单的了
        if (strcmp(sendMsgBuff, "QUIT") == 0 || strcmp(sendMsgBuff, "quit") == 0) { //退出
            flag = false; //结束循环的标识
            shutdown(clientfd, SHUT_WR); //这里注意哈，client主动与server断开连接，server会返回一个确认结束的信息哈
                                         //如果没有这一句的话recvMsg那边会一直卡在recv那里，程序无法结束 这里可以想一想，是个小坑
            return;
        }
        //接收
        int send_len = send(clientfd,sendMsgBuff,strlen(sendMsgBuff),0);
        if(send_len < 0){ //错误处理
            flag = false;
            shutdown(clientfd, SHUT_WR);
            perror("send error");
            return;
        }

    }
    //清空buf
    memset(sendMsgBuff, 0, BUF_SIZE);
    return;
}

void recvMsg(int clientfd){
    //清空buf
    memset(recvMsgBuff, 0, BUF_SIZE);
    while(flag){
        // 接收
        int recv_len = recv(clientfd,recvMsgBuff,BUF_SIZE,0);
        if(recv_len < 0){
            flag = false;
            perror("recv error");
            return;
        }else if(recv_len==0){ // 断开连接了
            cout<<"服务端断开连接了，清输入quit退出"<<endl;
            flag = false;
            return;
        }else{
            recvMsgBuff[recv_len] = '\0'; //字符串结尾
            cout<<recvMsgBuff<<endl;      //输出到终端
        }
    }
    //清空buf
    memset(recvMsgBuff, 0, BUF_SIZE);
    return;
}

int main(){
    //创建socket
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0){
        perror("socket error");
        return -1;
    }
    //服务器地址
    struct sockaddr_in serverAddr;  //总得知道自己要连接的目标吧
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9999);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //连接
    if(connect(sockfd,(struct sockaddr*)&serverAddr,sizeof(serverAddr))<0){
        perror("connect error");
        return -1;
    }else{
        cout<<"欢迎来到聊天室，清输入你的聊天室用户名！"<<endl;
    }

    //这里需要接收消息和发送消息，因此需要设置两个线程
    //循环发消息
    thread sendMsgThread(sendMsg,sockfd);
    //循环收消息
    thread recvMsgThread(recvMsg,sockfd);
    
    sendMsgThread.join();
    recvMsgThread.join();

    close(sockfd);

}
```

运行：
cd build

cmake ..

make

./server

./client

./client （多开几个终端，多个客户端连接）

不使用cmake也行 只要 编译 运行就好

git:https://github.com/zqzhang2023/zzqStudy


参考：

https://www.bilibili.com/video/BV1Q24y1v7Wa/?spm_id_from=333.337.search-card.all.click&vd_source=28887ecca1f25a715214067a2b3b58ac

https://blog.csdn.net/qq_44184756/article/details/130140778

 