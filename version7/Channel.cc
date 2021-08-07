#include"../base/Logging.h"

#include"Channel.h"
#include"Eventloop.h"

#include<poll.h>

using namespace muduo;

const int Channel::KNoneEvent=0;
const int Channel::KReadEvent=POLLIN|POLLPRI;
const int Channel::KWriteEvent=POLLOUT;

Channel::Channel(Eventloop* loop,int fd)
:loop_(loop),eventhandling_(false),fd_(fd),events_(0),revents_(0),index_(-1)
{
}

Channel::~Channel()
{
    assert(!eventhandling_);
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
    eventhandling_=true;
    if(revents_&POLLNVAL)//fd wasn't opended
    {
        LOG_WARN<<"Channel::handle_event() POLLNAVL";
    }
    if((revents_&POLLHUP)&&!(revents_&POLLIN))
    {
        LOG_WARN<<"Channel::handle_event() POLLHUP";
        if(closeCallback_){
            closeCallback_();
        }
    }
    if(revents_&(POLLERR|POLLNVAL))
    {
        if(errorCallback_)
        errorCallback_();
    }
    if(revents_&(POLLIN|POLLPRI|POLLHUP))
    {
        if(readCallback_)
        readCallback_(receiveTime);
    }
    if(revents_&POLLOUT)
    {
        if(writeCallback_)
        writeCallback_();
    }
    eventhandling_=false;
}
