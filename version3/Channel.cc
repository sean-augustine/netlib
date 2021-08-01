#include"../base/Logging.h"

#include"Channel.h"
#include"Eventloop.h"

#include<poll.h>

using namespace muduo;

const int Channel::KNoneEvent=0;
const int Channel::KReadEvent=POLLIN|POLLPRI;
const int Channel::KWriteEvent=POLLOUT;

Channel::Channel(Eventloop* loop,int fd)
:loop_(loop),fd_(fd),events_(0),revents_(0),index_(-1)
{
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::handleEvent()
{
    if(revents_&POLLNVAL)//fd wasn't opended
    {
        LOG_WARN<<"Channel::handle_event() POLLNAVL";
    }
    if(revents_&(POLLERR|POLLNVAL))
    {
        if(errorCallback_)
        errorCallback_();
    }
    if(revents_&(POLLIN|POLLPRI|POLLHUP))
    {
        if(readCallback_)
        readCallback_();
    }
    if(revents_&POLLOUT)
    {
        if(writeCallback_)
        writeCallback_();
    }
}
