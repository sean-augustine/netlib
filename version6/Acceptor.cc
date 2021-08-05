#include"Acceptor.h"
#include"Eventloop.h"
#include"InetAddress.h"
#include"Socketops.h"

#include<functional>

using namespace muduo;

Acceptor::Acceptor(Eventloop* loop,const InetAddress& listenaddr)
    :loop_(loop),
    acceptSocket_(sockets::CreateNoblocksockfd()),
    acceptfdChannel_(loop_,acceptSocket_.fd()),
    listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddress(listenaddr);
    acceptfdChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));
}

void Acceptor::listen()
{
    loop_->assertInloopThread();
    acceptSocket_.listen();
    listenning_=true;
    acceptfdChannel_.enableReading();
}

void Acceptor::handleRead()
{
    loop_->assertInloopThread();
    InetAddress peeraddr(0);
    
    int connfd=acceptSocket_.accept(&peeraddr);
    if(connfd>=0)
    {
        if(newConnCallback_)
        {
            newConnCallback_(connfd,peeraddr);//not good enough???transfer the fd by 'int'???
        }
        else 
            sockets::Close(connfd);
    }
}

