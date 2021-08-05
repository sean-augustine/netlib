#include"Socket.h"
#include"InetAddress.h"
#include"Socketops.h"

#include<string.h>

using namespace muduo;

Socket::~Socket()
{
    sockets::Close(sockfd_);
}

void Socket::bindAddress(const InetAddress& addr)
{
    sockets::Bind(sockfd_,addr.getSockaddr());
}

void Socket::listen()
{
    sockets::Listen(sockfd_);
}

int Socket::accept(InetAddress* peeraddr)
{
    struct sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    int connfd=sockets::Accept(sockfd_,&addr);
    if(connfd>0)
    {
        peeraddr->setSockaddr(addr);
    }
    return connfd;
}

void Socket::setReuseAddr(bool on)
{
    int flag=on?1:0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));
}