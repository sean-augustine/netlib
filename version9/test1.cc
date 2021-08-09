#include"Eventloop.h"
#include"Tcpclient.h"
#include"InetAddress.h"

#include<string>
#include<stdio.h>

static const std::string message="Hello!";

void onConnection(const muduo::Tcpconnectionptr& conn)
{
    if(conn->connected())
    {
        printf("onConnection:tid= %d new connection [%s] form [%s]\n",
        muduo::CurrentThread::tid(),
        conn->name().c_str(),conn->peerAddr().toHostPort().c_str());
        conn->send(message);
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

}

int main(int argc,char** argv)
{
    printf("main(): pid:%d\n",getpid());


    muduo::InetAddress serveraddr("127.0.0.1",8888);
    muduo::Eventloop loop;
    muduo::Tcpclient client(&loop,serveraddr);

    client.setConnCallback(onConnection);
    client.setMessCallback(onMessage);
    client.enableRetry();
    client.connect();

    
    loop.loop();
}