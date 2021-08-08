#ifndef TIMER_H
#define TIMER_H

#include"../base/Timestamp.h"

#include"Callbacks.h"
#include<boost/noncopyable.hpp>

namespace muduo
{
    class Timer: boost::noncopyable
    {
    public:
        Timer(const TimerCallback& cb,Timestamp when,double interval):callback_(cb),expiration_(when),interval_(interval),repeat_(interval>0.0)
        { }

        void run()
        {
            callback_();
        }

        Timestamp expiration()const {return expiration_;}
        bool repeat()const {return repeat_;}

        void restart(Timestamp now)
        {
            if(repeat_)
            {
                expiration_=addTime(now,interval_);
            }
            else
            {
                expiration_=Timestamp::invalid();//reset to 0s(since epoch)
            }
        }

    private:
        const TimerCallback callback_;
        Timestamp expiration_;//absolutly time to alarm
        const double interval_;
        const bool repeat_;
    };    
}

#endif