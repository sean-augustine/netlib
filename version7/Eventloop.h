#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include"../base/Thread.h"
#include"../base/Timestamp.h"
#include"../base/Mutex.h"

#include<vector>
#include<memory>
#include<functional>

#include"Callbacks.h"
#include"TimerId.h"

namespace muduo
{
    class Channel;
    class Poller;
    class TimerQueue;

class Eventloop:boost::noncopyable
{
public:
    typedef std::function<void()> functor;
    static Eventloop* getEventLoopOfCurrent();
    Eventloop();
    ~Eventloop();
    void loop();
    void quit();
    //for TimerQueue
    TimerId runAt(const Timestamp& time,const TimerCallback& cb);
    TimerId runAfter(double delay,const TimerCallback& cb);
    TimerId runEvery(double interval,const TimerCallback& cb);

    Timestamp pollReturnTime()const{return pollReturnTime_;}

    //run callback immediately in IO loop thread
    //it will wake_up the ::poll to return and run the cb
    //safe to call for other thread
    void runInloop(const functor& cb);//use to hanle the user events, arrange by the eventfd;
    //user in other thread to call runInloop,
    //cb will add into the queue and the IO thread will be waked up to run this cb
    void queueInloop(const functor& cb);
    void wakeup();

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    void assertInloopThread()//confirm current thread is iothread;
    {
        if(!isInloopthread())
        {
            abortInLoopThread();
        }
    }
    bool isInloopthread()
    {
        return threadId_==CurrentThread::tid();
    }

private:
    typedef std::vector<Channel*> ChannelList;

    void abortInLoopThread();//abort loop in IO thread;
    void handleRead();//wakeup, read eventfd
    void doPendingFunctors();

    ChannelList activeChannels_;
    std::unique_ptr<TimerQueue> timerQueue_;//for timer
    std::unique_ptr<Poller> poller_;//each eventloop has a unique poller
    Timestamp pollReturnTime_;
    bool quit_;
    bool looping_;//state value to indicate whether eventloop is in looping;
    const pid_t threadId_;//record the threadid who creat this eventloop;

    int wakeupFd_;
    bool callingPendingFuncs_;//is doing the pendingfunctors now?
    std::unique_ptr<Channel> wakeupChannel_;
    

    MutexLock mutex_;
    std::vector<functor> pendingFunctors;
};

}




#endif