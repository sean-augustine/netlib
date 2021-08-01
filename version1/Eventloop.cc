#include"Eventloop.h"
#include"../base/Logging.h"

#include<assert.h>
#include<poll.h>
using namespace muduo;

__thread Eventloop* t_loopInThisThread=0;


Eventloop::Eventloop():looping_(false),threadId_(CurrentThread::tid())
{
    LOG_TRACE<<"Eventloop creadted "<<this<<" in thread "<<threadId_;
    if(!t_loopInThisThread)
    {
        t_loopInThisThread=this;
    }
    else
    {
        LOG_FATAL<<"Another Eventloop "<<t_loopInThisThread<<" exits in this thread "<<threadId_;
    }
}

Eventloop::~Eventloop()
{
    assert(!looping_);
    t_loopInThisThread=NULL;
}

Eventloop* Eventloop::getEventLoopOfCurrent()
{
    return t_loopInThisThread;
}

void Eventloop::abortInLoopThread()
{
    LOG_FATAL<<"EventLoop::abortNoInLoopThread-Eventloop "
    <<this<<" was created in threadId_= "<<threadId_
    <<", current thread_id= "<<CurrentThread::tid();
}

void Eventloop::loop()//this loop do nothing
{
    assert(!looping_);
    assertInloopThread();
    looping_=true;
    ::poll(NULL,0,5*1000);
    looping_=false;
}
