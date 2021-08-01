#include"../base/Logging.h"

#include"Poller.h"
#include"Channel.h"

#include<poll.h>
#include<assert.h>

using namespace muduo;

Poller::Poller(Eventloop* loop):ownerloop_(loop)
{
}

Poller::~Poller()
{
}

//corn function

Timestamp Poller::poll(int timeoutMs,ChannelList* activeChannels)
{
    //activeChannel will be filled up in this body
    //pollfds_ should't change
    int numEvents=::poll(&*pollfds_.begin(),pollfds_.size(),timeoutMs);
    Timestamp now(Timestamp::now());//???
    if(numEvents>0)
    {
        LOG_TRACE<<numEvents<<" events happened";
        fillActiveChannel(numEvents,activeChannels);
    }
    else if(numEvents==0)
    {
        LOG_TRACE<<"nothing happend";
    }
    else
    {
        LOG_SYSERR<<"Poller::poll() error";
    }
    return now;//return the time when ::poll return
}

//can't do channel::handleEvent() when call the fillactivechannle
//because the handleEvent() will change the pollfds_,handleEvent() will be called after this function in IO_thread
//simplify the fillactivechannel to only multipexing the events not dispatch the events
void Poller::fillActiveChannel(int numevents,ChannelList* activeChannels)const 
{
    for(int i=0;i<pollfds_.size()&&numevents>0;++i)
    {
        if(pollfds_[i].revents>0)
        {
            --numevents;
            ChannelMap::const_iterator it=channels_.find(pollfds_[i].fd);
            assert(it!=channels_.end());
            Channel* channel=it->second;
            assert(channel->fd()==pollfds_[i].fd);
            channel->set_revents(pollfds_[i].revents);
            //pollfds_[i].revents=0;
            activeChannels->push_back(channel);
        }
    }
}

void Poller::updateChannel(Channel* channel)
{
    assertInLoopThread();
    LOG_TRACE<<"fd="<<channel->fd()<<" events="<<channel->events();
    if(channel->index()<0)//represent a new channel
    {
        assert(channels_.find(channel->fd())==channels_.end());
        struct pollfd pfd;
        pfd.events=channel->events();
        pfd.fd=channel->fd();
        pfd.revents=0;
        pollfds_.push_back(pfd);
        int idx=pollfds_.size()-1;//static_cast<int>(pollfds_.size())-1
        channels_[channel->fd()]=channel;
        channel->set_index(idx);
    }
    else
    {
        assert(channels_.find(channel->fd())!=channels_.end());
        assert(pollfds_[channel->index()].fd==channel->fd());
        int idx=channel->index();
        assert(idx>=0&&idx<pollfds_.size());
        struct pollfd pfd=pollfds_[idx];
        assert(pfd.fd==channel->fd()||pfd.fd==-1);//=-1 means temporary closed
        pfd.events=channel->events();//static_cast<short>(channel->events());
        pfd.revents=0;
        if(channel->isNoneEvent())
        {
            pfd.fd=-1;//???but when to open this fd???
        }
    }

}


