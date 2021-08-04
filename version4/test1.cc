#include"Eventloop.h"
#include"InetAddress.h"
#include"Socketops.h"
#include"Acceptor.h"

#include<stdio.h>


void newConnection(int sockfd,const muduo::InetAddress& peeraddr)
{
    printf("new connection from %s\n",peeraddr.toHostPort().c_str());
    ::write(sockfd,"how are you?\n",13);
    muduo::sockets::Close(sockfd);
}

int main()
{
    printf("main(): pid=%d \n",getpid());
    muduo::InetAddress addr(8888);
    muduo::Eventloop loop;
    
    muduo::Acceptor sendstr(&loop,addr);
    sendstr.setNewconnCallback(newConnection);
    sendstr.listen();
    loop.loop();
}