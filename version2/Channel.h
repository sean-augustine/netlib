#ifndef CHANNEL_H
#define CHANNEL_H

#include<boost/function.hpp>
#include<boost/noncopyable.hpp>


namespace muduo
{
    class Eventloop;

    class Channel:boost::noncopyable//each channel belong to one specifcal event and response one particular fd 
    {
    
    public:
        typedef boost::function<void()> EventCallback;

        Channel(Eventloop* loop,int fd);
        
        void handleEvent();
        
        void setReadCallback(const EventCallback& cb) {readCallback_=cb;}
        void setWeadCallback(const EventCallback& cb) {writeCallback_=cb;}
        void setEeadCallback(const EventCallback& cb) {errorCallback_=cb;}

        int fd() const {return fd_;}
        int events() const {return events_;}
        void set_revents(int revt) {revents_=revt;}
        bool isNoneEvent()const {return events_==KNoneEvent;}

        void enableReading(){events_|=KReadEvent;update();}
        void enableWriting(){events_|=KWriteEvent;update();}
        void disableWriting(){events_&=~KWriteEvent;update();}
        void disableAll(){events_=KNoneEvent;update();}

        //for poler
        int index(){return index_;}
        void set_index(int idx){index_=idx;}

        Eventloop* ownerLoop(){return loop_;}

    private:
        void update();
        static const int KNoneEvent;
        static const int KReadEvent;
        static const int KWriteEvent;
        Eventloop* loop_;//???destructor will destruct this pointer???
        const int fd_;
        int events_;
        int revents_;
        int index_;//used by poller;

        EventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback errorCallback_;
    };
}

#endif