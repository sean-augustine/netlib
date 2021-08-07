#include"Socketops.h"

#include"../base/Logging.h"

#include<errno.h>
#include<fcntl.h>
#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<unistd.h>

#define LISTENQ  1024

namespace muduo
{
    typedef struct sockaddr SA;
    void setNoblockCloseOnExec(int sockfd)
    {
        int flags=::fcntl(sockfd,F_GETFL,0);
        flags|=O_NONBLOCK;
        int ret=::fcntl(sockfd,F_SETFL,flags);
        if(ret==-1)
        {
            LOG_SYSERR<<"fcntl error";
        }

        flags=::fcntl(sockfd,F_GETFD,0);
        flags|=O_CLOEXEC;
        ret=::fcntl(sockfd,F_SETFD,flags);
        if(ret==-1)
        {
            LOG_SYSERR<<"fcntl error";
        }
    }
}

using namespace muduo;

int sockets::CreateNoblocksockfd()
{
    int sockfd=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(sockfd<0)
    {
        LOG_SYSFATAL<<"socket creat error";
    }
    setNoblockCloseOnExec(sockfd);
    return sockfd;
}

void sockets::Bind(int sockfd,const struct sockaddr_in& addr)
{
    int ret=::bind(sockfd,(SA*)(&addr),sizeof(addr));
    if(ret<0)
    {
        LOG_SYSFATAL<<"sockets::bind error";
    }
}

void sockets::Listen(int sockfd)
{
    int ret=::listen(sockfd,LISTENQ);
    if(ret<0)
    {
        LOG_SYSFATAL<<"sockets::listen error";
    }
}

int sockets::Accept(int sockfd,struct sockaddr_in* addr)
{
    socklen_t len=sizeof(*addr);
    int connfd=::accept(sockfd,(SA*)(addr),&len);
    
    if(connfd<0)
    {
        int savedError=errno;
        LOG_SYSERR<<"sockets::accept error";
        switch(savedError)
        {
            case EAGAIN:
            case EINTR:
            case ECONNABORTED:
            case EPROTO:
                errno=savedError;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                LOG_FATAL<<"unexpected error of ::accept "<<savedError;
                break;
            default:
                LOG_FATAL<<"unknow error of ::accept "<<savedError;
                break;
        }
    }
    else
    {
        setNoblockCloseOnExec(connfd);
    }
    return connfd;
}

void sockets::Close(int sockfd)
{
    if(::close(sockfd)<0)
    {
        LOG_SYSERR<<"sockets::close error";
    }
}

void sockets::shutdownWrite(int sockfd)
{
    if(::shutdown(sockfd,SHUT_WR)<0)
    {
        LOG_SYSERR<<"sockets::shutdownWrite error";
    }
}

void sockets::toHostPort(char* buf,size_t size,const struct sockaddr_in& addr)//change the sockaddr to ip:prot
{
    char host[INET_ADDRSTRLEN]="INVALID";
    ::inet_ntop(AF_INET,&addr.sin_addr,host,INET_ADDRSTRLEN);
    uint16_t port=sockets::nettoHost16(addr.sin_port);
    snprintf(buf,size,"%s:%u",host,port);
}

void sockets::formHostPort(const char* ip,uint16_t port,struct sockaddr_in* addr)//changet the ip:port to a sockaddr
{
    addr->sin_family=AF_INET;
    addr->sin_port=sockets::hosetoNet16(port);
    if(::inet_pton(AF_INET,ip,&(addr->sin_addr))<=0)
    {
        LOG_SYSERR<<"sockets::formHostport";
    }
}

struct sockaddr_in sockets::getLocalAddr(int sockfd)
{
    sockaddr_in localaddr;
    socklen_t len=sizeof(localaddr);
    bzero(&localaddr,len);
    int ret=::getsockname(sockfd,(SA*)&localaddr,&len);
    if(ret<0)
    {
        LOG_SYSERR<<"sockets::getLocalAddr->getsockname error";
    }
    return localaddr;
}

int sockets::getSocketerr(int sockfd)
{
    int optval;
    socklen_t len=sizeof(optval);
    if(::getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&optval,&len)<0)
    {
        return errno;
    }
    else
    return optval;
}