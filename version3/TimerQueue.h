#ifndef TIMEQUEUE_H
#define TIMEQUEUE_H

#include<boost/noncopyable.hpp>

#include"../base/Timestamp.h"
#include"Callbacks.h"
#include"Channel.h"


#include<vector>
#include<map>
#include<utility>
#include<memory>


namespace muduo
{
    class Eventloop;//should include Eventloop.h in .cc file
    class Timer;
    class TimerId;

    class TimerQueue: boost::noncopyable
    {
    public:
        TimerQueue(Eventloop*);
        ~TimerQueue();
        TimerId addTimer(const TimerCallback& cb,Timestamp when,double interval);//used by Eventloop
        //void cancel(TimerId TimerId);

    private:
        typedef std::unique_ptr<Timer> Timer_;
        typedef std::pair<Timestamp,Timer_> Entry;
        typedef std::map<Timestamp,Timer_> TimerList;

        void hanleRead();//called when timerfd alarms
        void reset(const std::vector<Entry>& expired,Timestamp now);//reset timer into timers_ according to interval>0
        std::vector<Entry> getExpired(Timestamp now);

        bool insert(Timer_);//insert timer into timers_,return (is this timer the earlist timer)

        Eventloop* loop_;
        const int timerfd_;
        Channel timerfdChannel_;
        TimerList timers_;//equivalent to some functions with time characteristic
    };

}

#endif
