#include"../base/Logging.h"

#include"Buffer.h"
//#include"Socketops.h"

#include<errno.h>
//#include<memory.h>
#include<sys/uio.h>//readv

using namespace muduo;

ssize_t Buffer::readFd(int fd,int* savedErr)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writeable=writableBytes();
    vec[0].iov_base=writeptr();
    vec[0].iov_len=writeable;
    vec[1].iov_base=extrabuf;
    vec[1].iov_len=sizeof(extrabuf);
    const ssize_t n=readv(fd,vec,2);
    if(n<0)
    {
        *savedErr=errno;
    }
    else if(implicit_cast<size_t>(n)<=writeable)
    {
        updateWriteIdx(n);
    }
    else
    {
        writeIndex_=buffer_.size();
        append(extrabuf,n-writeable);
    }
    return n;
}
