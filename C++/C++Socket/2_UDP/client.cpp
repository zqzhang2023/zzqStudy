#include<iostream>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<cstring>
#include<ctime>
using namespace std;

#define BUF_LEN  100

int main(){

    int clientFd;
    char Buf[BUF_LEN] = {0};
    struct sockaddr_in serverAddr;  //总得知道自己要连接的目标吧

    // 创建套接字
    clientFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientFd < 0) {
        cout<<"创建套接字失败!"<<endl;
        return -1;
    }

    //服务器地址
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(8080);

    //控制while循环，如果要一直不停的监听就直接while(true)就行了，根据业务功能做复杂的控制
    int whileCotr = 1;
    while(whileCotr!=0){
        whileCotr--;

        cout<<"请输入一个字符串，发送给服务端："<<endl;
        cin>>Buf;
        //发送数据到服务端
        int send_len = sendto(clientFd,Buf,strlen(Buf),0, (struct sockaddr*)&serverAddr,sizeof(serverAddr));
        if(send_len < 0){
            cout<<"发送数据失败!"<<endl;
            close(clientFd);
            return -1;
        }
        //重置缓冲区
        memset(Buf, 0, BUF_LEN);

		int recv_len = recvfrom(clientFd, Buf, BUF_LEN, 0,NULL,NULL);
        if(recv_len < 0){
            cout<<"接受数据失败!"<<endl;
            close(clientFd);
            return -1;
        }

        cout<<"服务端发送过来的数据为:"<<endl;
        cout<<Buf<<endl;
        
        memset(Buf, 0, BUF_LEN);   // 重置缓冲区
    }

    close(clientFd);

    return 0;
}