#include"Eventloop.h"
#include"InetAddress.h"
#include"Socketops.h"
#include"Acceptor.h"

#include<stdio.h>
#include<time.h>

#define MAXLINE 1024

void daytime(int sockfd,const muduo::InetAddress& peeraddr)
{
    char buf[MAXLINE];
    printf("new connection from %s\n",peeraddr.toHostPort().c_str());
    time_t ticks=time(NULL);
    snprintf(buf,sizeof(buf),"%.24s\n",ctime(&ticks));
    ::write(sockfd,buf,strlen(buf));
    muduo::sockets::Close(sockfd);
}

int main()
{
    printf("main(): pid=%d \n",getpid());
    muduo::InetAddress addr(8888);
    muduo::Eventloop loop;
    
    muduo::Acceptor sendstr(&loop,addr);
    sendstr.setNewconnCallback(daytime);
    sendstr.listen();

    loop.loop();

}