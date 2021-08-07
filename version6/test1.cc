#include"Tcpserver.h"
#include"InetAddress.h"
#include"Eventloop.h"
#include"Buffer.h"

#include<stdio.h>

void onConnection(const muduo::Tcpconnectionptr& tcpConn)
{
    if(tcpConn->connected())
    {
        printf("onConnection(): new connection [%s] form %s\n",
        tcpConn->name().c_str(),tcpConn->peerAddr().toHostPort().c_str());
    }
    else
    {
        printf("onConnection(): connection [%s] is down\n",tcpConn->name().c_str());
    }
}

void onMessage(const muduo::Tcpconnectionptr& tcpConn,muduo::Buffer* buf,muduo::Timestamp receiveTime)
{
    printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
    buf->readableBytes(),tcpConn->name().c_str(),receiveTime.toFormattedString().c_str());
    printf("onMassage(): [%s]\n",buf->getalldataAsstring().c_str());
}

int main()
{
    printf("main(): pid=%d\n",getpid());
    muduo::Eventloop loop;
    muduo::InetAddress addr(8888);
    muduo::Tcpserver server(&loop,addr);
    server.setConnCallback(onConnection);
    server.setMessCallback(onMessage);
    server.start();

    loop.loop();
}

