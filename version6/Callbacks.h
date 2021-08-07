#ifndef CALLBACKS_H
#define CALLBACKS_H



#include<functional>
#include<memory>

#include"../base/Timestamp.h"

namespace muduo
{
    class Tcpconnection;
    class Buffer;
    typedef std::shared_ptr<Tcpconnection> Tcpconnectionptr;

    typedef std::function<void()> TimerCallback;
    
    typedef std::function<void(const Tcpconnectionptr&)> ConnectionCallback;
    typedef std::function<void(const Tcpconnectionptr&,Buffer*,Timestamp)> MessageCallback;//timstamp is poll return time
    typedef std::function<void(const Tcpconnectionptr&)> CloseCallback;

}

#endif