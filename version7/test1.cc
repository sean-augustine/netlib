#include"Eventloop.h"
#include"Tcpserver.h"
#include"InetAddress.h"

#include<string>
#include<stdio.h>

static std::string message;


void onConnection(const muduo::Tcpconnectionptr& conn)
{
    if(conn->connected())
    {
        printf("onConnection:new connection [%s] form [%s]\n",
        conn->name().c_str(),conn->peerAddr().toHostPort().c_str());
        conn->send(message);
    }
    else//Tcpconnection::handleclose->closecallback->Tcpserver::removeconn->Tcpconnection::conndestory
    {
        printf("onConnection(): connection [%s] is down\n",conn->name().c_str());
    }
}

void onWriComplete(const muduo::Tcpconnectionptr& conn)
{
    conn->send(message);
}

void onMessage(const muduo::Tcpconnectionptr& tcpConn,muduo::Buffer* buf,muduo::Timestamp receiveTime)
{
    printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
    buf->readableBytes(),tcpConn->name().c_str(),receiveTime.toFormattedString().c_str());
    printf("onMassage(): [%s]\n",buf->getalldataAsstring().c_str());

    buf->reset();
}

int main()
{
    printf("main(): pid:%d\n",getpid());

    std::string line;
    for(int i=33;i<127;++i)
    {
        line.push_back(char(i));
    }
    line+=line;

    for(size_t i=0;i<127-33;++i)
    {
        message+=line.substr(i,72)+"\n";
    }

    muduo::InetAddress addr(8888);
    muduo::Eventloop loop;

    muduo::Tcpserver server(&loop,addr);
    server.setConnCallback(onConnection);
    server.setMessCallback(onMessage);
    server.setWriCompleteCallback(onWriComplete);
    server.start();
    
    loop.loop();
}