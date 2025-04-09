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