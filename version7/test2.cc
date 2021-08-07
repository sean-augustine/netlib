#include"Eventloop.h"
#include"Tcpserver.h"
#include"InetAddress.h"

#include<string>
#include<stdio.h>

static std::string message1;
static std::string message2;

void onConnection(const muduo::Tcpconnectionptr& conn)
{
    if(conn->connected())
    {
        printf("onConnection:tid= %d new connection [%s] form [%s]\n",
        muduo::CurrentThread::tid(),
        conn->name().c_str(),conn->peerAddr().toHostPort().c_str());
        conn->send(message1);
        conn->send(message2);
        conn->shutdown();
    }
    else//Tcpconnection::handleclose->closecallback->Tcpserver::removeconn->Tcpconnection::conndestory
    {
        printf("onConnection():tid= %d connection [%s] is down\n",muduo::CurrentThread::tid(),conn->name().c_str());
    }
}

void onMessage(const muduo::Tcpconnectionptr& tcpConn,muduo::Buffer* buf,muduo::Timestamp receiveTime)
{
    printf("onMessage(): tid= %d, received %zd bytes from connection [%s] at %s\n",
    muduo::CurrentThread::tid(),
    buf->readableBytes(),tcpConn->name().c_str(),receiveTime.toFormattedString().c_str());
    printf("onMassage(): [%s]\n",buf->getalldataAsstring().c_str());

    buf->reset();
}

int main(int argc,char** argv)
{
    printf("main(): pid:%d\n",getpid());
    int len1=100;
    int len2=100;

    message1.resize(len1);
    message2.resize(len2);

    std::fill(message1.begin(),message1.end(),'A');
    std::fill(message2.begin(),message2.end(),'B');

    muduo::InetAddress addr(8888);
    muduo::Eventloop loop;

    muduo::Tcpserver server(&loop,addr);
    server.setConnCallback(onConnection);
    server.setMessCallback(onMessage);
    if(argc>1)
    {
        server.setThreadnum(atoi(argv[1]));
    }
    server.start();
    
    loop.loop();
}