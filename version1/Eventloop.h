#ifndef EVENTLOOOP_H
#define EVENTLOOP_H

#include"../base/Thread.h"

namespace muduo
{

class Eventloop:boost::noncopyable
{
private:
    void abortInLoopThread();//abort loop in IO thread;
    bool looping_;//state value to indicate whether eventloop is in looping;
    const pid_t threadId_;//record the threadid who creat this eventloop;
public:
    static Eventloop* getEventLoopOfCurrent();
    Eventloop();
    ~Eventloop();
    void loop();
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