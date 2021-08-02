#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include"../base/Thread.h"
#include"../base/Timestamp.h"

#include<vector>
#include<memory>

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
    static Eventloop* getEventLoopOfCurrent();
    Eventloop();
    ~Eventloop();
    void loop();
    void quit();
    //+++
    TimerId runAt(const Timestamp& time,const TimerCallback& cb);
    TimerId runAfter(double delay,const TimerCallback& cb);
    TimerId runEvery(double interval,const TimerCallback& cb);

    Timestamp pollReturnTime()const{return pollReturnTime_;}

    void updateChannel(Channel* channel);
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

    ChannelList activeChannels_;
    std::unique_ptr<TimerQueue> timerqueue_;//+++
    std::unique_ptr<Poller> poller_;//each eventloop has a unique poller
    Timestamp pollReturnTime_;
    bool quit_;
    bool looping_;//state value to indicate whether eventloop is in looping;
    const pid_t threadId_;//record the threadid who creat this eventloop;
};

}




#endif