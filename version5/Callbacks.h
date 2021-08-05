#ifndef CALLBACKS_H
#define CALLBACKS_H

#include<functional>
#include<memory>

#include"../base/Timestamp.h"

namespace muduo
{
    class Tcpconnection;
    typedef std::shared_ptr<Tcpconnection> Tcpconnectionptr;

    typedef std::function<void()> TimerCallback;
    
    typedef std::function<void(const Tcpconnectionptr&)> ConnectionCallback;
    typedef std::function<void(const Tcpconnectionptr&,const char* data,ssize_t len)> MessageCallback;
    typedef std::function<void(const Tcpconnectionptr&)> CloseCallback;

}

#endif