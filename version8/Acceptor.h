#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include"Channel.h"
#include"Socket.h"

#include<boost/noncopyable.hpp>
#include<functional>


namespace muduo
{
    class Eventloop;
    class InetAddress;
    class Acceptor: boost::noncopyable//internal class, used by Tcpserver
    {
    //used to create new connnection and use callback to inform the user
    public:
        typedef std::function<void(int sockfd,const InetAddress&)> NewConnCallback;
        Acceptor(Eventloop* loop,const InetAddress& listenaddr);

        void setNewconnCallback(const NewConnCallback& cb){newConnCallback_=cb;}

        bool listenning(){return listenning_;}
        void listen();
    private:
        void handleRead();

        Eventloop* loop_;
        Socket acceptSocket_;
        Channel acceptfdChannel_;
        NewConnCallback newConnCallback_;
        bool listenning_;

    };
}

#endif