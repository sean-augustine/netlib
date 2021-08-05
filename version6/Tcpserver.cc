#include"Tcpserver.h"
#include"Eventloop.h"
#include"Acceptor.h"
#include"Socketops.h"

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
    snprintf(buf,sizeof(buf),"%d",nextConnId_);
    ++nextConnId_;
    std::string connectname=name_+buf;
    LOG_INFO<<"Tcpserver::newConnection: ["<<name_<<"]-new connction ["<<connectname
    <<"] form ["<<peeraddr.toHostPort()<<"]";

    InetAddress localaddr(sockets::getLocalAddr(sockfd));
    //initialize the tcpconnectionptr
    Tcpconnectionptr conn(new Tcpconnection(loop_,connectname,sockfd,localaddr,peeraddr));

    connections_[connectname]=conn;
    conn->setConnCallback(connectionCallback_);
    conn->setMessCallback(messageCallback_);
    conn->setCloseCallback(std::bind(&Tcpserver::removeConnection,this,_1));
    conn->connectEatablished();
}

void Tcpserver::removeConnection(const Tcpconnectionptr& conn)
{
    loop_->assertInloopThread();
    LOG_INFO<<"Tcpserver::removeConnection ["<<name_<<"]-conection "<<conn->name();
    size_t n=connections_.erase(conn->name());
    assert(n==1);
    loop_->queueInloop(std::bind(&Tcpconnection::connectDestory,conn));
}

