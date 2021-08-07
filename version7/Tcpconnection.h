#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include"InetAddress.h"
#include"Callbacks.h"
#include"Buffer.h"

#include<boost/noncopyable.hpp>
#include<string>

namespace muduo
{
    class Eventloop;
    class Channel;
    class Socket;
    class Tcpconnection: boost::noncopyable,
                         public std::enable_shared_from_this<Tcpconnection>
    {
    public:
        Tcpconnection(Eventloop* loop,const std::string connName,int sockfd,
        const InetAddress& localaddr,const InetAddress& peeraddr);
        ~Tcpconnection();

        Eventloop* getloop(){return loop_;}
        const std::string& name(){return name_;}
        const InetAddress& localAddr(){return localAddr_;}
        const InetAddress& peerAddr(){return peerAddr_;}
        bool connected()const{return state_==Kconnected;}

        void send(const std::string& message);//thread safe
        void shutdown();//thread safe

        void setTcpNodelay(bool on);
        void setTcpKeepalive(bool on);

        void setConnCallback(const ConnectionCallback& cb){connectionCallback_=cb;}
        void setMessCallback(const MessageCallback& cb){messageCallback_=cb;}
        void setCloseCallback(const CloseCallback& cb){closeCallback_=cb;}
        void setWriCompleteCallback(const WriteCompleteCallback& cb){writeCompleteCallback_=cb;}
        void connectEstablished();
        void connectDestory();
    private:
        enum State{Kconnecting,Kconnected,Kdisconnecting,Kdisconnected};
        void setState(State s){state_=s;}

        void handleRead(Timestamp);
        void handleWrite();
        void handleError();
        void handleClose();

        void sendInloop(const std::string& message);
        void shutdownInloop();

        Eventloop* loop_;
        std::string name_;
        std::unique_ptr<Channel> channel_;//??????
        std::unique_ptr<Socket> socket_;//??????
        InetAddress localAddr_;
        InetAddress peerAddr_;
        State state_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        CloseCallback closeCallback_;

        Buffer inputBuffer_;
        Buffer outputBuffer_;
    };
    
}

#endif