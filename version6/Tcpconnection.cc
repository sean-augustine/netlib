#include"../base/Logging.h"

#include"Tcpconnection.h"
#include"Eventloop.h"
#include"Channel.h"
#include"Socket.h"
#include"Socketops.h"

#include<stdio.h>
#include<errno.h>


using namespace muduo;

Tcpconnection::Tcpconnection(Eventloop* loop,const std::string connName,int sockfd,
const InetAddress& localaddr,const InetAddress& peeraddr)
    :loop_(loop),
    name_(connName),
    state_(Kconnecting),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop,sockfd)),
    localAddr_(localaddr),
    peerAddr_(peeraddr)
{
    LOG_DEBUG<<"Tcpconnectio::ctor[ "<<name_<<" ]at "<<this<<" fd= "<<sockfd;
    channel_->setReadCallback(std::bind(&Tcpconnection::handleRead,this));
}
Tcpconnection::~Tcpconnection()
{
    LOG_DEBUG<<"TcpConnection::dtor["<<name_<<"] at"<<this<<" fd="<<socket_->fd();
}

void Tcpconnection::connectEatablished()//put the connfd to the event loop, handle listenfd events
{
    loop_->assertInloopThread();
    assert(state_==Kconnecting);
    setState(Kconnected);
    channel_->enableReading();
    //when comes to new connection,this function will be called
    connectionCallback_(shared_from_this());
}

void Tcpconnection::connectDestory()
{
    loop_->assertInloopThread();
    assert(state_==Kconnected);\
    setState(Kdisconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());//??????
    loop_->removeChannel(channel_.get());
}

void Tcpconnection::handleRead()//handle connfd events
{
    char buf[65536];
    ssize_t n=::read(channel_->fd(),buf,sizeof(buf));
    //when connfd readable, this function will called
    if(n>0)
    {
        messageCallback_(shared_from_this(),buf,n);
    }
    else if(n==0)
    {
        handleClose();
    }
    else
    {
        handleError();
    }
    
}

void Tcpconnection::handleWrite()
{}

void Tcpconnection::handleClose()
{
    loop_->assertInloopThread();
    assert(state_==Kconnected);
    channel_->disableAll();//set events(channel) to NONE
    closeCallback_(shared_from_this());//bind to Tcpserver::removeConnection
}

void Tcpconnection::handleError()
{
    int err=sockets::getSocketerr(socket_->fd());
    LOG_ERROR<<"Tcpconnection::handleError: ["<<name_
    <<"] -SO_ERROR = "<<err<<" "<<strerror_tl(err);
}

