#ifndef INETADDRESS_H
#define INETADDRESS_H

#include"../base/copyable.h"

#include<string>

#include<netinet/in.h>

namespace muduo
{
    class InetAddress: public muduo::copyable
    {
    public:
        //constructor
        explicit InetAddress(uint16_t port);//mostly used in Tcpserver listening
        InetAddress(const std::string&ip, uint16_t port);
        //mostly used when accepting new connections
        InetAddress(const struct sockaddr_in& addr)
        :addr_(addr)
        {}
        std::string toHostPort()const;//change sockaddr to ip:port

        const struct sockaddr_in& getSockaddr()const
        {
            return addr_;
        }
        void setSockaddr(const struct sockaddr_in& addr){addr_=addr;}
    private:
        struct sockaddr_in addr_;
    };
    
}

#endif