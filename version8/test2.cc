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

    }
    else//Tcpconnection::handleclose->closecallback->Tcpserver::removeconn->Tcpconnection::conndestory
    {
        printf("onConnection():tid= %d connection [%s] is down\n",muduo::CurrentThread::tid(),conn->name().c_str());
    }
}



int main(int argc,char** argv)
{
    printf("main(): pid:%d\n",getpid());

    muduo::InetAddress addr(9981);
    muduo::Eventloop loop;

    muduo::Tcpserver server(&loop,addr);
    server.setConnCallback(onConnection);

    if(argc>1)
    {
        server.setThreadnum(atoi(argv[1]));
    }
    server.start();
    
    loop.loop();
}