#include<iostream>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<cstring>
#include<ctime>
using namespace std;

#define BUF_LEN  100

int main(){

    int serverFd;                     //定义服务端和客户端的文件描述符
    struct sockaddr_in serverAddr,clientAddr;  //定义服务端和客户端的地址
    socklen_t addrLength = sizeof(clientAddr); //serverAddr和clientAddr大小 后面传参要用
    char Buf[BUF_LEN] = {0};                   //定义接受缓冲区的buf	
    int optval = 1;

    //创建套接字
    serverFd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
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

    cout<<"服务器启动，等待客户端数据..."<<endl;

    int whileCotr = 1;
    while(whileCotr!=0){
        whileCotr--;

        int recv_len = recvfrom(serverFd,Buf,BUF_LEN,0,(struct sockaddr*)&clientAddr,&addrLength);

        if(recv_len<0){
            cout<<"接受客户端的数据失败!"<<endl;
            close(serverFd);
            return -1;
        }

        cout<<"客户端发送过来的数据为:"<<endl;
        cout<<Buf<<endl;

        int send_len = sendto(serverFd, Buf, strlen(Buf), 0, (struct sockaddr*)&clientAddr, addrLength);

        if(send_len<0){
            cout<<"发送数据失败!"<<endl;
            close(serverFd);
            return -1;
        }

        memset(Buf, 0, BUF_LEN); 
    }

    close(serverFd);

    return 0;
}