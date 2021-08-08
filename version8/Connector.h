#ifndef CONNECTOR_H
#define CONNECTOR_H

#include"InetAddress.h"
#include"TimerId.h"

#include<memory>
#include<functional>

namespace muduo
{
    class Eventloop;
    class Channel;
    class Connector
    {
    
    public:
        typedef std::function<void(int sockfd)> NewconnectionCallback;
        Connector(Eventloop* loop,const InetAddress& serverAddr);
        ~Connector();
        void setNewconnCallback(const NewconnectionCallback& cb){newConnectionCallback_=cb;}
        
        void start();//can be called in any thread
        void restart();//must be called in loop thread
        void stop();//can be called in any thread
    private:
        enum State {Kdisconneted,Kconnecting,Kconnected};
        static const int KMaxRetryDelayMs=30000;
        static const int KInitRetryDelayMs=500;

        void setState(State s){state_=s;}
        void startInloop();
        void connect();
        void connecting(int sockfd);
        void handleWrite();
        void handleError();
        void retry(int sockfd);
        int removeAndResetChannel();
        void resetChannel();
        /* data */
        Eventloop* loop_;
        InetAddress serverAddr_;
        bool connect_;
        State state_;
        std::unique_ptr<Channel> channel_;
        NewconnectionCallback newConnectionCallback_;
        int retryDelayMs_;
        TimerId timerId_;
    };

    typedef std::shared_ptr<Connector> ConnectorPtr;
}

#endif