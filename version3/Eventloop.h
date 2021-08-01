#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include"../base/Thread.h"

#include<vector>
#include<boost/scoped_ptr.hpp>

namespace muduo
{
    class Channel;
    class Poller;

class Eventloop:boost::noncopyable
{
private:
    typedef std::vector<Channel*> ChannelList;

    void abortInLoopThread();//abort loop in IO thread;

    ChannelList activeChannels_;
    boost::scoped_ptr<Poller> poller_;//each eventloop has a unique poller
    bool quit_;
    bool looping_;//state value to indicate whether eventloop is in looping;
    const pid_t threadId_;//record the threadid who creat this eventloop;
public:
    static Eventloop* getEventLoopOfCurrent();
    Eventloop();
    ~Eventloop();
    void loop();
    void quit();
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
    
};

}




#endif