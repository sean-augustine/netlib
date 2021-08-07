#include"../base/Logging.h"

#include"Eventloop.h"
#include"Channel.h"
#include"Poller.h"
#include"TimerQueue.h"

#include<assert.h>
#include<sys/eventfd.h>
#include<signal.h>

using namespace muduo;


class IgnoreSigpipe
{
    public:
    IgnoreSigpipe()
    {
        ::signal(SIGPIPE,SIG_IGN);
    }
};

IgnoreSigpipe initobj;


__thread Eventloop* t_loopInThisThread=0;
const int KpollTimeMs=10000;

static int creatEventfd()
{
    int evfd=::eventfd(0,EFD_NONBLOCK|EFD_CLOEXEC);
    if(evfd<0)
    {
        LOG_SYSERR<<"Eventfd error";
        abort();
    }
    return evfd;
}

Eventloop::Eventloop():
    looping_(false),
    quit_(false),
    callingPendingFuncs_(false),
    poller_(new Poller(this)),
    threadId_(CurrentThread::tid()),
    wakeupFd_(creatEventfd()),
    wakeupChannel_(new Channel(this,wakeupFd_))
{
    timerQueue_.reset(new TimerQueue(this));
    LOG_TRACE<<"Eventloop creadted "<<this<<" in thread "<<threadId_;
    if(!t_loopInThisThread)
    {
        t_loopInThisThread=this;
    }
    else
    {
        LOG_FATAL<<"Another Eventloop "<<t_loopInThisThread<<" exits in this thread "<<threadId_;
    }
    wakeupChannel_->setReadCallback(std::bind(&Eventloop::handleRead,this));
    wakeupChannel_->enableReading();//add to event loop(::poll)

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

void Eventloop::runInloop(const functor& cb)
{
    if(isInloopthread())
    {
        cb();
    }
    else
    queueInloop(cb);
}

void Eventloop::queueInloop(const functor& cb)
{
    MutexLockGuard lock(mutex_);
    pendingFunctors.push_back(cb);//if the Eventloop is doing event callback, there is no need to wakeup the wakeupfd
    if(!isInloopthread()||callingPendingFuncs_)//maybe not callling by runInloop()
    {
        wakeup();
    }
}

void Eventloop::wakeup()//write eventfd
{
    uint64_t one=1;
    ssize_t n=::write(wakeupFd_,&one,sizeof(one));
    if(n!=sizeof(one))
    {
        LOG_ERROR<<"Eventloop::wakeup() writes "<<n<<" bytes intead of 8";
    }
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
    timerQueue_->addTimer(cb,time,0.0);
}

TimerId Eventloop::runAfter(double delay,const TimerCallback& cb)
{
    Timestamp time(addTime(Timestamp::now(),delay));
    return runAt(time,cb);
}

TimerId Eventloop::runEvery(double interval,const TimerCallback& cb)
{
    Timestamp time(addTime(Timestamp::now(),interval));
    return timerQueue_->addTimer(cb,time,interval);
}

void Eventloop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop()==this);
    assertInloopThread();
    poller_->updateChannel(channel);
}

void Eventloop::removeChannel(Channel* channel)
{
    assert(channel->ownerLoop()==this);
    assertInloopThread();
    poller_->removeChannel(channel);
}

void Eventloop::quit()
{
    quit_=true;
    if(!isInloopthread())
    {
        wakeup();
    }
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
            (*it)->handleEvent(pollReturnTime_);
        }
        doPendingFunctors();
    }
    LOG_TRACE<<"Eventloop "<<this<<" stop looping";
    looping_=false;
}

void Eventloop::handleRead()//wakeup, read eventfd
{
    uint64_t one=1;
    ssize_t n=::read(wakeupFd_,&one,sizeof(one));
    if(n!=sizeof(one))
    {
        LOG_ERROR<<"Eventloop::handleRead read "<<n<<" bytes instead of 8";
    }
}

void Eventloop::doPendingFunctors()
{
    std::vector<functor> functors;
    callingPendingFuncs_ = true;
    mutex_.lock();
    functors.swap(pendingFunctors);
    mutex_.unlock();
    for(functor c:functors)
    {
        c();
    }
    callingPendingFuncs_=false;
}