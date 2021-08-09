#ifndef TIMEQUEUE_H
#define TIMEQUEUE_H

#include<boost/noncopyable.hpp>

#include"../base/Timestamp.h"
#include"Callbacks.h"
#include"Channel.h"


#include<vector>
#include<set>
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
        void cancel(TimerId);

    private:
        typedef std::shared_ptr<Timer> Timerptr;
        typedef std::pair<Timestamp,Timerptr> Entry;
        typedef std::set<Entry> TimerList;

        void addTimerInloop(Timerptr);
        void cancelInloop(TimerId);
        void handleRead();//called when timerfd alarms
        void reset(std::vector<Entry>& expired,Timestamp now);//reset timer into timers_ according to interval>0
        std::vector<Entry> getExpired(Timestamp now);

        bool insert(Timerptr);//insert timer into timers_,return (is this timer the earlist timer)

        Eventloop* loop_;
        const int timerfd_;
        Channel timerfdChannel_;
        TimerList timers_;//equivalent to some functions with time characteristic
    };

}
#endif
