#include"Tcpserver.h"
#include"Eventloop.h"
#include"Acceptor.h"
#include"Socketops.h"
#include"EventloopThreadPool.h"

#include"../base/Logging.h"

#include<stdio.h>

using std::placeholders::_1;
using std::placeholders::_2;

using namespace muduo;

Tcpserver::Tcpserver(Eventloop* loop,const InetAddress& listenaddr)
    :loop_(loop),
    name_(listenaddr.toHostPort()),
    acceptor_(new Acceptor(loop_,listenaddr)),
    started_(false),
    threadPool_(new EventloopThreadPool(loop)),
    nextConnId_(1)
{
    acceptor_->setNewconnCallback(std::bind(&Tcpserver::newConnection,this,_1,_2));
}
Tcpserver::~Tcpserver()
{
}
void Tcpserver::start()//add listenfd to IO_thread event loop
{
    if(!started_)
    {
        started_=true;
        threadPool_->start();
    }
    if(!acceptor_->listenning())
    {
        loop_->runInloop(std::bind(&Acceptor::listen,acceptor_.get()));//make sure listen in IO_thread loop
    }
}

void Tcpserver::newConnection(int sockfd,const InetAddress& peeraddr)//sockfd is a connfd
{
    loop_->assertInloopThread();
    char buf[32];
    snprintf(buf,sizeof(buf),"/%d",nextConnId_);
    ++nextConnId_;
    std::string connectname=name_+buf;
    LOG_INFO<<"Tcpserver::newConnection: ["<<name_<<"]-new connction ["<<connectname
    <<"] form ["<<peeraddr.toHostPort()<<"]";

    InetAddress localaddr(sockets::getLocalAddr(sockfd));
    //initialize the tcpconnectionptr
    Eventloop* ioloop=threadPool_->getnextLoop();
    Tcpconnectionptr conn(new Tcpconnection(ioloop,connectname,sockfd,localaddr,peeraddr));

    connections_[connectname]=conn;
    conn->setConnCallback(connectionCallback_);
    conn->setMessCallback(messageCallback_);
    conn->setWriCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&Tcpserver::removeConnection,this,_1));
    
    ioloop->runInloop(std::bind(&Tcpconnection::connectEstablished,conn));
}

void Tcpserver::removeConnection(const Tcpconnectionptr& conn)
{
    loop_->runInloop(std::bind(&Tcpserver::removeConnInloop,this,conn));
}

void Tcpserver::removeConnInloop(const Tcpconnectionptr& conn)
{
    loop_->assertInloopThread();
    LOG_INFO<<"Tcpserver::removeConnectionInloop ["<<name_<<"]-conection "<<conn->name();
    size_t n=connections_.erase(conn->name());
    assert(n==1);
    Eventloop* ioloop=conn->getloop();
    ioloop->queueInloop(std::bind(&Tcpconnection::connectDestory,conn));
}



void Tcpserver::setThreadnum(int threadNum)
{
    assert(threadNum>=0);
    threadPool_->setThreadNum(threadNum);
}
