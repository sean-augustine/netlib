#include"../base/Logging.h"

#include"Socket.h"
#include"InetAddress.h"
#include"Socketops.h"

#include<string.h>
#include<netinet/tcp.h>

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

void Socket::shutdownWrite()
{
    sockets::shutdownWrite(sockfd_);
}

void Socket::setTcpNoDelay(bool on)
{
    int optval=on?1:0;
    int ret=::setsockopt(sockfd_,IPPROTO_TCP,TCP_NODELAY,&optval,sizeof(optval));
    if(ret<0)
    {
        LOG_SYSERR<<"Socket::setTcpNodelay->setsockopt error";
    }
}

void Socket::setTcpKeepalive(bool on)
{
    int optval=on?1:0;
    int ret=::setsockopt(sockfd_,SOL_SOCKET,SO_KEEPALIVE,&optval,sizeof(optval));
    if(ret<0)
    {
        LOG_SYSERR<<"Socket::setTcpKeepalive->setsockopt error";
    }
}