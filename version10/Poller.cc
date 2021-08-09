#include"../base/Logging.h"

#include"Poller.h"
#include"Channel.h"

#include<poll.h>
#include<sys/epoll.h>
#include<assert.h>
#include<algorithm>

using namespace muduo;


//POLLER
Poller::Poller(Eventloop* loop):ownerloop_(loop)
{
}

Poller::~Poller()=default;

//PPOLLER
PPoller::PPoller(Eventloop* loop):Poller(loop)
{
}

PPoller::~PPoller()=default;
Timestamp PPoller::poll(int timeoutMs,ChannelList* activeChannels)
{
    //activeChannel will be filled up in this body
    //pollfds_ should't change
    int numEvents=::poll(&*pollfds_.begin(),pollfds_.size(),timeoutMs);//change vector into array;
    Timestamp now(Timestamp::now());//as return_time for Eventloop::pollreturntime
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
void PPoller::fillActiveChannel(int numevents,ChannelList* activeChannels)const 
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

void PPoller::updateChannel(Channel* channel)
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
        channels_[channel->fd()]=channel;//add into map
        channel->set_index(idx);//correspond to the index in pollfds_
    }
    else
    {
        assert(channels_.find(channel->fd())!=channels_.end());
        assert(channels_[channel->fd()]==channel);
        int idx=channel->index();
        assert(idx>=0&&idx<pollfds_.size());
        struct pollfd& pfd=pollfds_[idx];
        assert(pfd.fd==channel->fd()||pfd.fd==(-channel->fd()-1));//=-1 means temporary closed
        pfd.events=channel->events();//static_cast<short>(channel->events());
        pfd.revents=0;
        if(channel->isNoneEvent())
        {
            pfd.fd=-channel->fd()-1;//???but when to open this fd???
        }
    }

}

void PPoller::removeChannel(Channel* channel)
{
    assertInLoopThread();
    LOG_TRACE<<"fd= "<<channel->fd();
    assert((channels_.find(channel->fd()))!=channels_.end());
    assert(channels_[channel->fd()]==channel);
    assert(channel->isNoneEvent());
    int idx=channel->index();
    assert((idx>0)&&(idx<pollfds_.size()));
    const struct pollfd& pfd=pollfds_[idx];
    assert(pfd.fd==(-channel->fd()-1)&&pfd.events==channel->events());
    size_t n=channels_.erase(channel->fd());
    assert(n==1);
    if(idx==pollfds_.size()-1)
    pollfds_.pop_back();
    else
    {
        int tempfd=pollfds_.back().fd;
        std::iter_swap(pollfds_.begin()+idx,pollfds_.end()-1);
        if(tempfd<0)
        {
            tempfd=-tempfd-1;
        }
        channels_[tempfd]->set_index(idx);
        pollfds_.pop_back();
    }
}

//EPOLLER

namespace
{
    const int Knew=-1;
    const int Kadded=1;
    const int Kdelete=2;  
}


EPoller::EPoller(Eventloop* loop)
    :Poller(loop),
    epollFd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(KinitEventSize)
{
    if(epollFd_<0)
    {
        LOG_FATAL<<"EPoller::epll_creat1 error";
    }
}

EPoller::~EPoller()
{
    ::close(epollFd_);
}

Timestamp EPoller::poll(int timeoutMs,ChannelList* activeChannels)
{
    int nums=::epoll_wait(epollFd_,events_.data(),static_cast<int>(events_.size()),
    timeoutMs);
    Timestamp now(Timestamp::now());
    if(nums>0)
    {
        LOG_TRACE<<nums<<" events happend";
        fillActiveChannel(nums,activeChannels);
        if(nums==events_.size())
        {
            events_.resize(2*nums);
        }
    }
    else if(nums==0)
    {
        LOG_TRACE<<"nothing happened";
    }
    else{
        LOG_SYSERR<<"EPoller::poll->epoll_wait error";
    }
    return now;
}

void EPoller::fillActiveChannel(int eventNums,ChannelList* activeChannels) const
{
    assert(eventNums<=events_.size());
    for(int i=0;i<eventNums;++i)
    {
        Channel* channel=static_cast<Channel*>(events_[i].data.ptr);
        int fd=channel->fd();
        ChannelMap::const_iterator it=channels_.find(fd);
        assert(it!=channels_.end());
        assert(it->second==channel);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EPoller::updateChannel(Channel* channel)
{
    assertInLoopThread();
    LOG_TRACE<<"fd = "<<channel->fd()<<" events = "<<channel->events();
    const int idx=channel->index();
    if(idx==Knew||idx==Kdelete)
    {
        int fd=channel->fd();
        if(idx==Knew)
        {
            assert(channels_.find(fd)==channels_.end());
            channels_[fd]=channel;
        }
        else
        {
            assert(channels_.find(fd)!=channels_.end());
            assert(channels_[fd]==channel);
        }
        channel->set_index(Kadded);
        update(EPOLL_CTL_ADD,channel);//??????
    }
    else
    {
        int fd=channel->fd();
        assert(channels_.find(fd)!=channels_.end());
        assert(channels_[fd]==channel);
        assert(idx==Kadded);
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL,channel);
            channel->set_index(Kdelete);
        }
        else
        {
            update(EPOLL_CTL_MOD,channel);
        }
    }
}

void EPoller::update(int operation,Channel* channel)
{
    struct epoll_event event;
    bzero(&event,sizeof(event));
    event.data.ptr=(void*)channel;
    event.events=channel->events();
    int fd=channel->fd();
    if(::epoll_ctl(epollFd_,operation,fd,&event)<0)
    {
        if(operation==EPOLL_CTL_DEL)
        {
            LOG_SYSERR<<"epoll_ctl op = EPOLL_CTL_DEL, fd ="<<fd;
        }
        else
        {
            LOG_SYSERR<<"epoll_ctl op = EPOLL_CTL_MOD, fd ="<<fd;
        }
    }
}

void EPoller::removeChannel(Channel* channel)
{
    assertInLoopThread();
    int fd=channel->fd();
    LOG_TRACE<<"fd = "<<fd;
    assert(channels_.find(fd)!=channels_.end());
    assert(channels_[fd]==channel);
    assert(channel->isNoneEvent());
    int idx=channel->index();
    assert(idx==Kadded||idx==Kdelete);
    size_t n=channels_.erase(fd);
    assert(n==1);
    if(idx==Kadded)
    {
        update(EPOLL_CTL_DEL,channel);
    }
    channel->set_index(Knew);
}



