#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include"../base/Thread.h"

#include"../base/Condition.h"

#include<vector>
#include<boost/ptr_container/ptr_vector.hpp>
#include<boost/noncopyable.hpp>

namespace muduo
{
    class Eventloop;
    class EventloopThread;
    class EventloopThreadPool:boost::noncopyable
    {
    public:
        EventloopThreadPool(Eventloop* baseloop);
        ~EventloopThreadPool();
        void setThreadNum(int num){numThreads=num;}
        void start();
        Eventloop* getnextLoop();
    private:
        /* data */
        Eventloop* baseloop_;
        bool started_;
        int numThreads;
        int next_;//always in loop thread
        boost::ptr_vector<EventloopThread> threads_;
        std::vector<Eventloop*> loops_;
    };
    
}


#endif