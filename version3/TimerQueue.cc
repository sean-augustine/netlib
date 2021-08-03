#include"../base/Logging.h"

#include"TimerQueue.h"
#include"Eventloop.h"
#include"TimerId.h"
#include"Timer.h"

#include<assert.h>
#include<sys/timerfd.h>
#include<map>



namespace muduo
{
namespace timer
{
    int creatTimerfd()
    {
        int timerfd = ::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK|TFD_CLOEXEC);
        if(timerfd<0)
        {
            LOG_SYSERR<<"timer_creat() error";
        }
        return timerfd;
    }
    struct timespec howmuchtimefromnow(Timestamp when)//to get relatively time from absolutly time
    {
        int64_t us=when.microSecondsSinceEpoch()-Timestamp::now().microSecondsSinceEpoch();//caculate the relatively time
        if(us<100)
        {
            us=100;
        }
        struct timespec ts;
        ts.tv_sec=static_cast<time_t>(us/Timestamp::kMicroSecondsPerSecond);
        ts.tv_nsec=static_cast<long>((us%Timestamp::kMicroSecondsPerSecond)*1000);
        return ts;//ns, used to set time in timefd;
    }
    void readTimefd(int timefd,Timestamp now)
    {
        uint64_t res;
        ssize_t n=::read(timefd,&res,sizeof(res));
        LOG_TRACE<<"TimerQueue::handleread() "<<res<<" at "<<now.toString();
        if(n!=sizeof(res))
        {
            LOG_ERROR<<"TimerQueue::handleRead() reads "<<n<<" bytes instead of 8";
        }
    }

    void resetTimerfd(int timerfd,Timestamp expiration)
    {
        struct itimerspec newvalue;
        struct itimerspec oldvalue;
        bzero(&newvalue,sizeof(newvalue));
        bzero(&oldvalue,sizeof(oldvalue));
        newvalue.it_value =howmuchtimefromnow(expiration);
        int ret=::timerfd_settime(timerfd,0,&newvalue,&oldvalue);
        if(ret)//if ret==0 means success
        {
            LOG_ERROR<<"timerfd_settime() error";
        }
    }


}
}

using namespace muduo;
using namespace muduo::timer;

TimerQueue::TimerQueue(Eventloop* loop):
    loop_(loop),
    timerfd_(creatTimerfd()),
    timerfdChannel_(loop,timerfd_),
    timers_()
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead,this));//initialize the channel status
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
    ::close(timerfd_);
}

TimerId TimerQueue::addTimer(const TimerCallback& cb,Timestamp when,double interval)
{
    Timerptr ptr(new Timer(cb,when,interval));
    loop_->assertInloopThread();
    bool earlistchange = insert(ptr);
    if(earlistchange)
    {
        resetTimerfd(timerfd_,when);
    }
    return TimerId(ptr.get());
}

bool TimerQueue::insert(Timerptr timerptr)
{
    bool earlistchange=false;
    Timestamp when=timerptr->expiration();
    auto it =timers_.begin();
    if(it==timers_.end()||when<it->first)
    {
        earlistchange=true;
    }
    timers_.insert(std::make_pair(when,timerptr));
    return earlistchange;
}

void TimerQueue::handleRead()
{
    loop_->assertInloopThread();
    Timestamp now(Timestamp::now());
    readTimefd(timerfd_,now);
    std::vector<Entry> expired=getExpired(now);
    for(auto it=expired.begin();it!=expired.end();++it)
    {
        it->second->run();
    }
    reset(expired,now);//for repeat
}

void TimerQueue::reset(std::vector<Entry>& expired,Timestamp now)
{
    Timestamp nextExprie;
    for(auto it=expired.begin();it!=expired.end();++it)
    {
        if(it->second->repeat())
        {
            it->second->restart(now);
            insert(it->second);
        }
    }
    if(!timers_.empty())
    {
        nextExprie=timers_.begin()->first;
    }
    if(nextExprie.valid())
    {
        resetTimerfd(timerfd_,nextExprie);
    }
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expire;
    TimerList::iterator it=timers_.upper_bound(now);
    assert(it==timers_.end()||now<it->first);
    std::copy(timers_.begin(),it,back_inserter(expire));
    timers_.erase(timers_.begin(),it);
    return expire;
}
