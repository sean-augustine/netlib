#include"Connector.h"
#include"Channel.h"
#include"Eventloop.h"
#include"Socketops.h"


#include"../base/Logging.h"

#include<error.h>

using namespace muduo;

const int Connector::KMaxRetryDelayMs;

Connector::Connector(Eventloop* loop,const InetAddress& serverAddr)
    :loop_(loop),
    serverAddr_(serverAddr),
    connect_(false),
    state_(Kdisconneted),
    retryDelayMs_(KInitRetryDelayMs),
    timerId_(NULL)
{
    LOG_DEBUG<<"ctor["<<this<<"]";
}

Connector::~Connector()
{
    LOG_DEBUG<<"dtor["<<this<<"]";
    loop_->cancel(timerId_);//??????logout timer
    assert(!channel_);//confirm the channel has been delete
}

void Connector::start()
{
    connect_=true;
    loop_->runInloop(std::bind(&Connector::startInloop,this));
}

void Connector::startInloop()
{
    loop_->assertInloopThread();
    assert(state_==Kdisconneted);
    if(connect_)
    {
        connect();
    }
    else{
        LOG_DEBUG<<"do not connect";
    }
}

void Connector::connect()
{
    int sockfd=sockets::CreateNoblocksockfd();
    int ret=sockets::Connect(sockfd,serverAddr_.getSockaddr());
    int savedErr=(ret==0)?0:errno;
    switch (savedErr)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting(sockfd);
            break;

        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sockfd);
            break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            LOG_SYSERR<<"connect error in Connetor::startInloop "<<savedErr;
            sockets::Close(sockfd);
            break;
        
        default:
            LOG_SYSERR<<"connect error in Connetor::startInloop "<<savedErr;
            sockets::Close(sockfd);
            break;
    }

}

void Connector::restart()
{
    loop_->assertInloopThread();
    setState(Kdisconneted);
    retryDelayMs_=KInitRetryDelayMs;
    connect_=true;
    startInloop();
}

void Connector::stop()
{
    connect_=false;
    loop_->cancel(timerId_);//logout timer
}

void Connector::connecting(int sockfd)//get a channel and put into loop
{
    setState(Kconnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_,sockfd));
    channel_->setWriteCallback(std::bind(&Connector::handleWrite,this));
    channel_->setErrorCallback(std::bind(&Connector::handleError,this));

    channel_->enableWriting();
}

int Connector::removeAndResetChannel()//called in handleWrite
{
    channel_->disableAll();
    loop_->removeChannel(channel_.get());
    int sockfd=channel_->fd();
    loop_->queueInloop(std::bind(&Connector::resetChannel,this));
    return sockfd;
}

void Connector::resetChannel()
{
    channel_.reset();//release channel object
}

void Connector::handleWrite()//start()->startInloop()->connect()->connecting()->setCallback
{
    LOG_TRACE<<"Connector::handleWrite "<<state_;
    if(state_==Kconnecting)
    {
        int sockfd=removeAndResetChannel();//cancel checking this sockfd whatever it gets err or being connected
        int err=sockets::getSocketerr(sockfd);
        if(err)//connect err
        {
            LOG_WARN<<"Connector::handleWrite-SO_ERROR = "<<err<<" "<<strerror_tl(err);
            retry(sockfd);//will close this sockfd
        }
        else if(sockets::isSelfConnect(sockfd))//self connect
        {
            LOG_WARN<<"Connector::handleWrite-Self Connect";
            retry(sockfd);
        }
        else//connect normally;
        {
            setState(Kconnected);
            if(connect_)
            {
                newConnectionCallback_(sockfd);
            }
            else
            {
                sockets::Close(sockfd);
            }
        }

    }
    else
    {
        assert(state_==Kdisconneted);//??????
    }
}

void Connector::handleError()
{
    LOG_ERROR<<"Connctor::handleError";
    assert(state_==Kconnecting);
    int sockfd=removeAndResetChannel();
    int err=sockets::getSocketerr(sockfd);
    LOG_TRACE<<"SO_ERROR= "<<err<<" "<<strerror_tl(err);
    retry(sockfd);
}

void Connector::retry(int sockfd)
{
    sockets::Close(sockfd);
    setState(Kdisconneted);
    if(connect_)
    {
        LOG_INFO<<"Connetor::retry- retry connecting to "<<serverAddr_.toHostPort()<<" in "
        <<retryDelayMs_<< "ms. ";
        timerId_=loop_->runAfter(retryDelayMs_/1000.0,std::bind(&Connector::startInloop,this));//??????
        retryDelayMs_=std::min(retryDelayMs_*2,KMaxRetryDelayMs);
    }
    else 
    {
        LOG_DEBUG<<"Connector stoped";
    }
}


