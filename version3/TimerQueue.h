#ifndef TIMEQUEUE_H
#define TIMEQUEUQ_H

#include<boost/noncopyable.hpp>
#include<boost/scoped_ptr.hpp>

#include"../base/Timestamp.h"

#include<vector>
#include<set>
#include<utility>


namespace muduo
{
    class Eventloop;//should include Eventloop.h in .cc file
    class Timer;
    class TimerId;
    class Channel;

    class TimerQueue: boost::noncopyable
    {
    public:
        TimerQueue(Eventloop*);
        ~TimerQueue();
        TimerId addTimer();//used by Eventloop
        //void cancel(TimerId TimerId);

    private:
        typedef boost::scoped_ptr<Timer> unique_Timer;
        typedef std::pair<Timestamp,unique_Timer> Entry;
        typedef std::set<Entry> TimerList;

        void hanleRead();//called when timerfd alarms
        void reset(const std::vector<Entry>& expired,Timestamp now);//??????
        std::vector<Entry> getExpired(Timestamp now);

        bool insert(Timer* timer);

        Eventloop* loop_;
        const int timerfd_;
        Channel timefdChannel_;
        TimerList timers_;


    };

}

#endif
