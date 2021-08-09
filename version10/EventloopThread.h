#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include"../base/Mutex.h"
#include"../base/Thread.h"
#include"../base/Condition.h"

#include<boost/noncopyable.hpp>

namespace muduo
{
    class Eventloop;

    class EventloopThread: boost::noncopyable
    {
    public:
        EventloopThread();
        ~EventloopThread();
        Eventloop* startloop();
    private:
        void ThreadFunc();

        Eventloop* loop_;
        bool exiting_;
        Thread thread_;
        MutexLock mutex_;
        Condition cond_;
    };
}

#endif