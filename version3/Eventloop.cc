#include"../base/Logging.h"

#include"Eventloop.h"
#include"Channel.h"
#include"Poller.h"
#include"TimerQueue.h"

#include<assert.h>

using namespace muduo;


__thread Eventloop* t_loopInThisThread=0;
const int KpollTimeMs=10000;

Eventloop::Eventloop():
    looping_(false),
    quit_(false),
    poller_(new Poller(this)),
    threadId_(CurrentThread::tid())
{
    timerqueue_.reset(new TimerQueue(this));
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

//timers

TimerId Eventloop::runAt(const Timestamp& time,const TimerCallback& cb)
{
    timerqueue_->addTimer(cb,time,0.0);
}

TimerId Eventloop::runAfter(double delay,const TimerCallback& cb)
{
    Timestamp time(addTime(Timestamp::now(),delay));
    return runAt(time,cb);
}

TimerId Eventloop::runEvery(double interval,const TimerCallback& cb)
{
    Timestamp time(addTime(Timestamp::now(),interval));
    return timerqueue_->addTimer(cb,time,interval);
}

void Eventloop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop()==this);
    assertInloopThread();
    poller_->updateChannel(channel);
}

void Eventloop::quit()
{
    quit_=true;
    //wakeup();
}

void Eventloop::loop()
{
    assert(!looping_);
    assertInloopThread();
    quit_=false;
    looping_=true;
    while(!quit_)
    {
        activeChannels_.clear();//clear up the active channel list;
        pollReturnTime_=poller_->poll(KpollTimeMs,&activeChannels_);
        for(ChannelList::iterator it=activeChannels_.begin();it!=activeChannels_.end();++it)
        {
            (*it)->handleEvent();
        }
    }
    LOG_TRACE<<"Eventloop "<<this<<" stop looping";
    looping_=false;
}
