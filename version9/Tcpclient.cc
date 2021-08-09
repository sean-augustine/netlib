#include"Tcpclient.h"
#include"Eventloop.h"
#include"Connector.h"
#include"Socketops.h"

#include"../base/Logging.h"

#include<stdio.h>
#include<string>

using namespace muduo;
using std::placeholders::_1;
namespace muduo
{
    namespace detail
    {
        void removeConnection(Eventloop* loop,const Tcpconnectionptr& conn)
        {
            loop->queueInloop(std::bind(&Tcpconnection::connectDestory,conn));
        }
    }
}


Tcpclient::Tcpclient(Eventloop* loop,const InetAddress& serverAddr)
    :loop_(loop),
    connector_(new Connector(loop_,serverAddr)),
    retry_(false),
    connect_(false),
    nextConnId_(1)
{
    connector_->setNewconnCallback(std::bind(&Tcpclient::newConnection,this,_1));
    LOG_INFO<<"Tcpclient ctor ["<<this<<"] -connector: "<<connector_.get();
}

Tcpclient::~Tcpclient()
{
    LOG_INFO<<"Tcpclient dtor ["<<this<<"] -connctor "<<connector_.get();
    Tcpconnectionptr conn;
    mutex_.lock();
    conn=tcpConnection_;
    mutex_.unlock();
    if(conn)
    {
        CloseCallback cb=std::bind(&detail::removeConnection,loop_,_1);
        loop_->runInloop(std::bind(&Tcpconnection::setCloseCallback,conn,cb));
    }
    else
    {
        connector_->stop();
    }
}

void Tcpclient::connect()
{
    LOG_INFO<<"Tcpclient::connect ["<<this<<"] -connecting to "
    <<connector_->serverAddr().toHostPort();
    connect_=true;
    connector_->start();//key action
}

void Tcpclient::disconnect()
{
    connect_=false;
    mutex_.lock();
    if(tcpConnection_)
    {
        tcpConnection_->shutdown();
    }
    mutex_.unlock();
}

void Tcpclient::stop()
{
    connect_=false;
    connector_->stop();
}

void Tcpclient::newConnection(int sockfd)
{
    loop_->assertInloopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    char buf[32];
    snprintf(buf,sizeof(buf),"%s-%d",localAddr.toHostPort().c_str(),nextConnId_++);
    string name_=buf;
    Tcpconnectionptr conn(new Tcpconnection(loop_,name_,sockfd,localAddr,peerAddr));
    conn->setConnCallback(connectionCallback_);
    conn->setMessCallback(messageCallback_);
    conn->setWriCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&Tcpclient::removeConnection,this,_1));
    mutex_.lock();
    tcpConnection_=conn;
    mutex_.unlock();
    conn->connectEstablished();
}

void Tcpclient::removeConnection(const Tcpconnectionptr& conn)
{
    loop_->assertInloopThread();
    assert(loop_==conn->getloop());
    mutex_.lock();
    assert(tcpConnection_==conn);
    tcpConnection_.reset();//clear the shared_ptr;
    mutex_.unlock();
    loop_->queueInloop(std::bind(&Tcpconnection::connectDestory,conn));
    if(retry_&&connect_)
    {
        LOG_INFO<<"Tcpclient::connect["<<this<<"] -reconnecting to"<<connector_->serverAddr().toHostPort();
        connector_->restart();
    }
}
