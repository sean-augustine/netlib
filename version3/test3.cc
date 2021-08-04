#include"Eventloop.h"
#include"EventloopThread.h"
#include<stdio.h>

void runInthread()
{
    printf("runInThread():pid=%d,tid=%d\n",getpid(),muduo::CurrentThread::tid());
}

int main()
{
    printf("main:pid=%d,tid=%d\n",getpid(),muduo::CurrentThread::tid());
    muduo::EventloopThread loopThread;
    muduo::Eventloop* loop=loopThread.startloop();
    loop->runInloop(runInthread);
    sleep(1);
    loop->runAfter(2,runInthread);
    sleep(3);
    loop->quit();

    printf("exit main().\n");
    return 1;
}