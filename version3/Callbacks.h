#ifndef CALLBACKS_H
#define CALLBACKS_H

#include<functional>

namespace muduo
{
    typedef std::function<void()> TimerCallback;
}

#endif