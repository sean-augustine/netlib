#include"Eventloop.h"
#include"../base/Thread.h"
#include<stdio.h>
/* s1
void Threadfunc()
{
    printf("threadfunc():pid= %d,tid= %d\n",getpid(),muduo::CurrentThread::tid());
    muduo::Eventloop loop;
    loop.loop();
}


int main()
{
    printf("main(): pif= %d, tid= %d\n",getpid(),muduo::CurrentThread::tid());
    muduo::Eventloop loop;
    muduo::Thread thread(Threadfunc);
    thread.start();

    loop.loop();
    pthread_exit(NULL);
}
*/
//s2
muduo::Eventloop* g_loop;
void threadfunc()
{
    g_loop->loop();
}

int main()
{
    muduo::Eventloop loop;
    g_loop=&loop;
    muduo::Thread t(threadfunc);
    t.start();
    t.join();

}