#include"Eventloop.h"
#include"InetAddress.h"
#include"Socketops.h"
#include"Acceptor.h"

#include<stdio.h>


void newConnection1(int sockfd,const muduo::InetAddress& peeraddr)
{
    printf("new connection from %s\n",peeraddr.toHostPort().c_str());
    ::write(sockfd,"how are you?\n",13);
    muduo::sockets::Close(sockfd);
}

void newConnection2(int sockfd,const muduo::InetAddress& peeraddr)
{
    printf("new connection from %s\n",peeraddr.toHostPort().c_str());
    ::write(sockfd,"who are you?\n",13);
    muduo::sockets::Close(sockfd);
}

int main()
{
    printf("main(): pid=%d \n",getpid());
    muduo::InetAddress addr1(8888);
    muduo::InetAddress addr2(8889);
    muduo::Eventloop loop;
    
    muduo::Acceptor sendstr1(&loop,addr1);
    muduo::Acceptor sendstr2(&loop,addr2);
    sendstr1.setNewconnCallback(newConnection1);
    sendstr2.setNewconnCallback(newConnection2);
    sendstr1.listen();
    sendstr2.listen();
    loop.loop();
}