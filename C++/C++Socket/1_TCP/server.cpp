#include<iostream>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<cstring>
#include<ctime>
using namespace std;

#define BUF_LEN  100

int main(){

    int serverFd,clientFd;                     //定义服务端和客户端的文件描述符
    struct sockaddr_in serverAddr,clientAddr;  //定义服务端和客户端的地址
    socklen_t addrLength = sizeof(clientAddr); //serverAddr和clientAddr大小 后面传参要用
    char Buf[BUF_LEN] = {0};                   //定义接受缓冲区的buf	
    int optval = 1;

    //创建套接字
    serverFd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (serverFd < 0) {
        cout<<"创建套接字失败！"<<endl;
        return -1;
    }

    //初始化地址
    serverAddr.sin_family = AF_INET; //IPV4
    serverAddr.sin_addr.s_addr = INADDR_ANY;  //自动获取IP地址
    serverAddr.sin_port = htons(8080);               //设置端口号

    //设置地址和端口号可以重复使用(非必须)
    if(setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0){
        cout<<"设置地址和端口号可以重复使用失败！"<<endl;
        close(serverFd);
        return -1;
    }

    //绑定操作
    if(bind(serverFd,(struct sockaddr*)&serverAddr,addrLength) < 0){
        cout<<"绑定失败！返回的失败编号为: -2"<<endl;
        close(serverFd);
        return -1;
    }

    //监听 //允许连接的最大值为10
    if(listen(serverFd,10) < 0){
        cout<<"监听失败！"<<endl;
        close(serverFd);
    }

    cout<<"等待客户端连接 accept 会阻塞线程"<<endl;

    //控制while循环，如果要一直不停的监听就直接while(true)就行了，根据业务功能做复杂的控制
    int whileCotr = 1;
    while(whileCotr!=0){
        whileCotr--;
        //接受连接
        clientFd = accept(serverFd,(struct sockaddr*)&clientAddr,&addrLength);
        if (clientFd < 0) {
            cout<<"接受连接失败!"<<endl;
            close(serverFd);
            return -1;
        }

        //接受客户端的返回数据 也会阻塞
        int recv_len = recv(clientFd,Buf,BUF_LEN,0);
        if(recv_len<0){
            cout<<"接受客户端的whileCotr数据失败!"<<endl;
            close(serverFd);
            close(clientFd);
            return -1;
        }

        cout<<"客户端发送过来的数据为:"<<endl;
        cout<<Buf<<endl;

        //发送数据
        int send_len = send(clientFd, Buf, recv_len, 0);
        if(send_len<0){
            cout<<"发送数据失败!"<<endl;
            close(serverFd);
            close(clientFd);
            return -1;
        }

        //关闭客户端套接字
        close(clientFd);
        //清空缓冲区
        memset(Buf, 0, BUF_LEN); 
    }

    close(serverFd);



    return 0;
}