#ifndef POLLER_H
#define POLLER_H

#include"../base/Timestamp.h"

#include"Eventloop.h"

#include<vector>
#include<map>

struct pollfd;

namespace muduo
{
    class Channel;
    class Poller:boost::noncopyable
    {
    public:
        typedef std::vector<Channel*> ChannelList;

        Poller(Eventloop*);
        ~Poller();//default

        //must be called in IO_thread()--below tow functions
        Timestamp poll(int timeoutMs,ChannelList* activeChannels);
        void updateChannel(Channel* channel);//triggered by update() in Channel class;
        void removeChannel(Channel* channel);
        
        void assertInLoopThread()
        {
            ownerloop_->assertInloopThread();
        }
    private:
        void fillActiveChannel(int numevents,ChannelList* activeChannels) const;
        typedef std::vector<struct pollfd> PollFdList;
        typedef std::map<int,Channel*> ChannelMap;//fd(in pollfd.fd)->channel*

        Eventloop* ownerloop_;
        PollFdList pollfds_;//used by ::poll(pollfd*,nfds,timeout)
        ChannelMap channels_;
    };

}

#endif