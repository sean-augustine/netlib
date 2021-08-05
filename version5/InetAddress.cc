#include"InetAddress.h"
#include"Socketops.h"

#include<string.h>

using namespace muduo;

static const in_addr_t InaddrAny=INADDR_ANY;

InetAddress::InetAddress(uint16_t port)
{
    bzero(&addr_,sizeof(addr_));
    addr_.sin_family=AF_INET;
    addr_.sin_addr.s_addr=sockets::hosetoNet32(INADDR_ANY);
    addr_.sin_port=sockets::hosetoNet16(port);
}

InetAddress::InetAddress(const std::string& ip,uint16_t port)
{
    bzero(&addr_,sizeof(addr_));
    sockets::formHostPort(ip.c_str(),port,&addr_);
}

std::string InetAddress::toHostPort()const
{
    char buf[32];
    sockets::toHostPort(buf,sizeof(buf),addr_);
    return buf;
}