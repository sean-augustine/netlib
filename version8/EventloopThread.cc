#include"EventloopThread.h"
#include"Eventloop.h"

#include<functional>

using namespace muduo;

EventloopThread::EventloopThread()
    :loop_(NULL),
    exiting_(false),
    thread_(std::bind(&EventloopThread::ThreadFunc,this)),
    mutex_(),
    cond_(mutex_)
{
}

EventloopThread::~EventloopThread()
{
    exiting_=true;
    loop_->quit();
    thread_.join();
}

Eventloop* EventloopThread::startloop()
{
    assert(!thread_.started());
    thread_.start();
    mutex_.lock();
    while(loop_==NULL)
    {
        cond_.wait();//wait for Threadfunc to create a eventloop
    }
    mutex_.unlock();
    return loop_;
}

void EventloopThread::ThreadFunc()
{
    Eventloop loop;
    mutex_.lock();
    loop_=&loop;
    cond_.signal();
    mutex_.unlock();
    loop.loop();
}