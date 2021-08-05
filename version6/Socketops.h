#ifndef SOCKETOPS_H
#define SOCKETOPS_H

#include<arpa/inet.h>

namespace muduo
{
    
    namespace sockets
    {
        inline uint32_t hosetoNet32(uint32_t host32)
        {
            return htonl(host32);
        }

        inline uint16_t hosetoNet16(uint16_t host16)
        {
            return htons(host16);
        }

        inline uint32_t nettoHost32(uint32_t net32)
        {
            return ntohl(net32);
        }

        inline uint16_t nettoHost16(uint16_t net16)
        {
            return ntohs(net16);
        }

        int CreateNoblocksockfd();
        void Bind(int sockfd,const struct sockaddr_in& addr);
        void Listen(int sockfd);
        int Accept(int sockfd,struct sockaddr_in* addr);
        void Close(int sockfd);

        void toHostPort(char* buf,size_t size,const struct sockaddr_in& addr);
        void formHostPort(const char* ip,uint16_t port,struct sockaddr_in* addr);

        struct sockaddr_in getLocalAddr(int sockfd);
        int getSocketerr(int sockfd);
    }
}


#endif