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