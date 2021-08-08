#include"EventloopThreadPool.h"
#include"Eventloop.h"
#include"EventloopThread.h"

using namespace muduo;

EventloopThreadPool::EventloopThreadPool(Eventloop* baseloop)
    :baseloop_(baseloop),
    started_(false),
    numThreads(0),
    next_(0)
{
}
EventloopThreadPool::~EventloopThreadPool()
{
}

void EventloopThreadPool::start()
{
    assert(!started_);
    baseloop_->assertInloopThread();
    started_=true;

    for(int i=0;i<numThreads;++i)
    {
        EventloopThread* t=new EventloopThread;
        threads_.push_back(t);
        loops_.push_back(t->startloop());
    }
    
}


Eventloop* EventloopThreadPool::getnextLoop()
{
    baseloop_->assertInloopThread();
    Eventloop* loop=baseloop_;
    if(!loops_.empty())
    {
        loop=loops_[next_++];
        if(next_>=loops_.size())
        {
            next_=0;
        }
    }
    return loop;
}