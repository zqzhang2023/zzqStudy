#ifndef INETADDRESS_H
#define INETADDRESS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

// 封装socket地址类型
class InetAddress {
public:
    //两个构造
    explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");
    explicit InetAddress(const sockaddr_in &addr):addr_(addr){};

    //获取IP
    std::string toIp() const;
    //获取IP与Port
    std::string toIpPort() const;
    //获取port
    uint16_t toPort() const;
    //直接获取sockaddr_in
    const sockaddr_in* getSockAddr() const {return &addr_;}
    void setSockAddr(const sockaddr_in &addr) { addr_ = addr; }
    
private:
    sockaddr_in addr_;
};

#endif