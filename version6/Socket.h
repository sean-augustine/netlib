#ifndef SOCKET_H
#define SOCKET_H
#include<boost/noncopyable.hpp>

namespace muduo
{
    class InetAddress;

    class Socket: boost::noncopyable
    {
    public:
        explicit Socket(int sockfd)
        :sockfd_(sockfd)
        {}
        ~Socket();
        int fd(){return sockfd_;}
        void bindAddress(const InetAddress& addr);
        void listen();
        int accept(InetAddress* peeraddr);
        void setReuseAddr(bool on);
        void shutdownWrite();
    private:
        const int sockfd_;
    };
}


#endif