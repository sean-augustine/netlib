#include"Eventloop.h"
#include"Connector.h"
#include"InetAddress.h"

#include<stdio.h>

muduo::Eventloop* g_loop;

void connCallback(int sockfd)
{
    printf("connected.\n");
}

int main()
{
    muduo::Eventloop loop;
    g_loop=&loop;
    muduo::InetAddress addr("127.0.0.1",9981);
    std::vector<muduo::ConnectorPtr> vec(10);
    for(int i=0;i<10;++i)
    {
        vec[i].reset(new muduo::Connector(&loop,addr));
        vec[i]->setNewconnCallback(connCallback);
        vec[i]->start();
    }

    loop.loop();
}