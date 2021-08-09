#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include"Tcpconnection.h"

#include"../base/Mutex.h"

#include"boost/noncopyable.hpp"

namespace muduo
{
    class Eventloop;
    class Connector;
    class Tcpclient:boost::noncopyable
    {
    public:
        typedef std::shared_ptr<Connector> Connectorptr;
        Tcpclient(Eventloop* loop,const InetAddress& serverAddr);
        ~Tcpclient();

        void connect();
        void disconnect();
        void stop();

        Tcpconnectionptr connection()const//get the connection
        {
            MutexLockGuard lock(mutex_);//will change the mutex_ even the function is const
            return tcpConnection_;
        }

        bool retry()const{return retry_;}
        void enableRetry(){retry_=true;}

        void setConnCallback(const ConnectionCallback& cb){connectionCallback_=cb;}
        void setMessCallback(const MessageCallback& cb){messageCallback_=cb;}
        void setWriCompleteCallback(const WriteCompleteCallback& cb){writeCompleteCallback_=cb;}
    private:
        void newConnection(int sockfd);
        void removeConnection(const Tcpconnectionptr& conn);
        /* data */
        Eventloop* loop_;
        Connectorptr connector_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        bool retry_;
        bool connect_;
        
        int nextConnId_;
        mutable MutexLock mutex_;
        Tcpconnectionptr tcpConnection_;
    };
    
}


#endif