#ifndef TCPSERVER_H
#define TCPSERVER_H

#include"Callbacks.h"
#include"Tcpconnection.h"

#include<boost/noncopyable.hpp>

#include<string>
#include<map>


namespace muduo
{
    class Eventloop;
    class Acceptor;
    class EventloopThreadPool;
    class Tcpserver: boost::noncopyable
    {
    public:
        Tcpserver(Eventloop* loop,const InetAddress& listenaddr);
        ~Tcpserver();

        void start();

        void setConnCallback(const ConnectionCallback& cb){connectionCallback_=cb;}
        void setMessCallback(const MessageCallback& cb){messageCallback_=cb;}
        void setWriCompleteCallback(const WriteCompleteCallback& cb){writeCompleteCallback_=cb;}

        void setThreadnum(int threadNum);

    private:
        void newConnection(int sockfd,const InetAddress& peer);//used for Acceptor
        void removeConnection(const Tcpconnectionptr& conn);
        void removeConnInloop(const Tcpconnectionptr& conn);
        typedef std::map<std::string,Tcpconnectionptr> ConnectionMap;
        Eventloop* loop_;
        const std::string name_;
        std::unique_ptr<Acceptor> acceptor_;//a Tcpserver onws only one acceptor
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        bool started_;
        int nextConnId_;//usde for a specific connected name
        ConnectionMap connections_;
        std::unique_ptr<EventloopThreadPool> threadPool_;
    };
}


#endif