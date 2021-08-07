#include"../base/Logging.h"

#include"Tcpconnection.h"
#include"Eventloop.h"
#include"Channel.h"
#include"Socket.h"
#include"Socketops.h"

#include<stdio.h>
#include<errno.h>


using namespace muduo;
using std::placeholders::_1;

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
    channel_->setReadCallback(std::bind(&Tcpconnection::handleRead,this,_1));
    channel_->setWriteCallback(std::bind(&Tcpconnection::handleWrite,this));
    channel_->setCloseCallback(std::bind(&Tcpconnection::handleClose,this));
    channel_->setErrorCallback(std::bind(&Tcpconnection::handleError,this));
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
    assert(state_==Kconnected||state_==Kdisconnecting);
    setState(Kdisconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());//??????
    loop_->removeChannel(channel_.get());
}

void Tcpconnection::handleRead(Timestamp receiveTime)//handle connfd events
{
    int savedErr=0;
    ssize_t n=inputBuffer_.readFd(channel_->fd(),&savedErr);
    //when connfd readable, this function will called
    if(n>0)
    {
        messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);
    }
    else if(n==0)
    {
        handleClose();
    }
    else
    {
        errno=savedErr;
        LOG_SYSERR<<"Tcpconnection::handleRead->buffer.readfd";
        handleError();
    }
    
}

void Tcpconnection::handleWrite()
{
    loop_->assertInloopThread();
    if(channel_->isWriting())
    {
        ssize_t n=::write(socket_->fd(),outputBuffer_.readptr(),outputBuffer_.readableBytes());
        if(n>0)
        {
            outputBuffer_.updateReadIdx(n);
            if(outputBuffer_.readableBytes()==0)
            {
                channel_->disableWriting();
                if(state_==Kdisconnecting)
                {
                    shutdownInloop();//surely in loop thread
                }
            }
            else
            {
                LOG_TRACE<<"need write next time";
            }
        }
        else
        {
            LOG_SYSERR<<"Tcpconnection::handlewrite->write error";
        }
    }
    else
    {
        LOG_TRACE<<"Connection is down, no more writing";
    }
}

void Tcpconnection::handleClose()
{
    loop_->assertInloopThread();
    LOG_TRACE<<"Tcpconnection::handleclose state= "<<state_;
    assert(state_==Kconnected||state_==Kdisconnecting);
    channel_->disableAll();//set events(channel) to NONE
    closeCallback_(shared_from_this());//bind to Tcpserver::removeConnection
}

void Tcpconnection::handleError()
{
    int err=sockets::getSocketerr(socket_->fd());
    LOG_ERROR<<"Tcpconnection::handleError: ["<<name_
    <<"] -SO_ERROR = "<<err<<" "<<strerror_tl(err);
}

void Tcpconnection::shutdown()
{
    if(state_==Kconnected)
    {
        setState(Kdisconnecting);
        loop_->runInloop(std::bind(&Tcpconnection::shutdownInloop,shared_from_this()));
    }
}

void Tcpconnection::shutdownInloop()
{
    loop_->assertInloopThread();
    if(!channel_->isWriting())
    {
        socket_->shutdownWrite();
    }
}

void Tcpconnection::send(const std::string& message)
{
    if(state_==Kconnected)
    {
        if(loop_->isInloopthread())
        {
            sendInloop(message);
        }
        else
        {
            loop_->runInloop(std::bind(&Tcpconnection::sendInloop,this,message));
        }
    }
}

void Tcpconnection::sendInloop(const std::string& message)
{
    loop_->assertInloopThread();
    ssize_t nwrote=0;
    if(!channel_->isWriting()&&outputBuffer_.readableBytes()==0)//don,t exit wait to send data before
    {
        nwrote=::write(channel_->fd(),message.data(),message.length());
        if(nwrote>=0)
        {
            if(nwrote<message.size())
            {
                LOG_TRACE<<"I am going to write more data";
            }
        }
        else
        {
            nwrote=0;
            if(errno!=EWOULDBLOCK)
            {
                LOG_SYSERR<<"Tcpconnection::sendInloop->write error";
            }
        }
    }
    assert(nwrote>=0);
    if(nwrote<message.size())
    {
        outputBuffer_.append(message.data()+nwrote,message.size()-nwrote);//put the remainning data into outputbuffer_
        if(!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }

}

