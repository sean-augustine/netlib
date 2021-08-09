#ifndef POLLER_H
#define POLLER_H

#include"../base/Timestamp.h"

#include"Eventloop.h"

#include<vector>
#include<map>

struct pollfd;
struct epoll_event;

namespace muduo
{
    class Channel;
    class Poller:boost::noncopyable
    {
    public:
        typedef std::vector<Channel*> ChannelList;
        Poller(Eventloop*);
        virtual ~Poller();

        virtual Timestamp poll(int timeoutMs,ChannelList* activeChannels)=0;
        virtual void updateChannel(Channel* channel)=0;//triggered by update() in Channel class;
        virtual void removeChannel(Channel* channel)=0;

        void assertInLoopThread()
        {
            ownerloop_->assertInloopThread();
        }

    protected:
        typedef std::map<int,Channel*> ChannelMap;
        ChannelMap channels_;
    private:
        Eventloop* ownerloop_;
    };


    class PPoller:public Poller
    {
    public:

        PPoller(Eventloop*);
        ~PPoller();//default

        //must be called in IO_thread()--below tow functions
        Timestamp poll(int timeoutMs,ChannelList* activeChannels) override;
        void updateChannel(Channel* channel) override;//triggered by update() in Channel class;
        void removeChannel(Channel* channel) override;

    private:
        void fillActiveChannel(int numevents,ChannelList* activeChannels) const;
        typedef std::vector<struct pollfd> PollFdList;
        PollFdList pollfds_;//used by ::poll(pollfd*,nfds,timeout)
    };

    class EPoller:public Poller
    {
        public:
        EPoller(Eventloop* loop);
        ~EPoller();

        Timestamp poll(int timeoutMs,ChannelList* acticvChannels) override;
        void updateChannel(Channel* channel) override;
        void removeChannel(Channel* channel) override;
        private:
        static const int KinitEventSize=16;
        void fillActiveChannel(int numevents,ChannelList* activeChannels) const;
        void update(int operation,Channel* channel);
        typedef std::vector<struct epoll_event> Eventlist;
        int epollFd_;
        Eventlist events_;
    };

}

#endif