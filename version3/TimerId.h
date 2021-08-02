#ifndef TIMERID_H
#define TIMERID_H

#include"../base/copyable.h"

namespace muduo
{
    class Timer;

    class TimerId: public muduo::copyable//used to save a timer pointer
    {
    public:
        explicit TimerId(Timer* timer):value_(timer)
        {}
    private:
        Timer* value_;
    };
    
}

#endif