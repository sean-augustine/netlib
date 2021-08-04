#ifndef CONDITION_H
#define CONDITION_H

#include"Mutex.h"

#include<pthread.h>
#include<boost/noncopyable.hpp>
#include<errno.h>

namespace muduo
{
    class Condition: boost::noncopyable
    {
    public:
        explicit Condition(MutexLock& mutex):mutex_(mutex)
        {
            pthread_cond_init(&cond,NULL);
        }
        ~Condition()
        {
            pthread_cond_destroy(&cond);
        }
        void wait()
        {
            pthread_cond_wait(&cond,mutex_.getPthreadMutex());
        }
        bool waitSecond(int seconds)//ture for timeout,false for others
        {
            timespec abstime;
            clock_gettime(CLOCK_REALTIME,&abstime);
            abstime.tv_sec+=seconds;
            return ETIMEDOUT==pthread_cond_timedwait(&cond,mutex_.getPthreadMutex(),&abstime);
        }
        void signal()
        {
            pthread_cond_signal(&cond);
        }
        void signalAll()
        {
            pthread_cond_broadcast(&cond);
        }
    private:
        pthread_cond_t cond;
        MutexLock& mutex_;
    };
}


#endif