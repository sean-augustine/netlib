#ifndef TIMERID_H
#define TIMERID_H

#include"../base/copyable.h"
#include<memory>

namespace muduo
{
    class Timer;

    class TimerId: public muduo::copyable//used to save a timer pointer
    {
    public:
        explicit TimerId(std::shared_ptr<Timer> timer):value_(timer)
        {}
    private:
        friend class TimerQueue;
        std::weak_ptr<Timer> value_;
    };
    
}

#endif