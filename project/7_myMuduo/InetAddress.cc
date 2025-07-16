#include "InetAddress.h"
#include <string.h>

//构造
InetAddress::InetAddress(uint16_t port, std::string ip ){
    bzero(&addr_,sizeof(addr_)); //将addr_清零
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
};
//获取IP
std::string InetAddress::toIp() const {
    // 得把addr_之中的网络字节序传成本地字节序
    char buf[64] = {0};
    // 网络地址从二进制形式转换为点分十进制（IPv4）或冒号十六进制（IPv6）的字符串形式
    inet_ntop(AF_INET,&addr_.sin_addr,buf,sizeof(buf));
    return buf;
};
//获取IP与Port
std::string InetAddress::toIpPort() const {
    char buf[64] = {0};
    inet_ntop(AF_INET,&addr_.sin_addr,buf,sizeof(buf));
    size_t end = strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf+end,":%u",port);
    return buf;
};
//获取port
uint16_t InetAddress::toPort() const {
    return ntohs(addr_.sin_port);
};

// #include<iostream>
// int main(){
//     InetAddress addr(8080);
//     std::cout<<addr.toIpPort()<<std::endl;
// }