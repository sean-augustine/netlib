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

TimerQueue::TimerQueue(Eventloop* loop):loop_(loop),timerfd_(creatTimerfd()),
timerfdChannel_(loop,timerfd_),timers_()
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::hanleRead,this));//initialize the channel status
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
    ::close(timerfd_);

}

TimerId TimerQueue::addTimer(const TimerCallback& cb,Timestamp when,double interval)
{
    Timer_ timerptr(new Timer(cb,when,interval));
    loop_->assertInloopThread();
    timers_.insert(std::make_pair(when,timerptr));
}


std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expire;
    TimerList::iterator it=timers_.upper_bound(now);
    std::copy(timers_.begin(),it,back_inserter(expire));
    timers_.erase(timers_.begin(),it);
    return expire;
}
