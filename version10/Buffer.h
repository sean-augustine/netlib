#ifndef BUFFER_H
#define BUFFER_H

#include"../base/copyable.h"

#include<vector>
#include<string>
#include<algorithm>

#include<assert.h>

namespace muduo
{
    class Buffer: public muduo::copyable
    {
    public:
        static const size_t KinitPrepend=8;
        static const size_t KinitialSize=1024;//in total data will have 1032 initial bytes size
        Buffer():
        buffer_(KinitialSize+KinitPrepend),
        readIndex_(KinitPrepend),
        writeIndex_(KinitPrepend)
        {
            assert(readableBytes()==0);
            assert(writableBytes()==KinitialSize);
            assert(prependableBytes()==KinitPrepend);
        }

        void swap(Buffer& rhs)
        {
            buffer_.swap(rhs.buffer_);//???why can visit private member directly???
            std::swap(readIndex_,rhs.readIndex_);
            std::swap(writeIndex_,rhs.writeIndex_);
        }

        size_t readableBytes()const
        {
            return writeIndex_-readIndex_;
        }
        size_t writableBytes()const
        {
            return buffer_.size()-writeIndex_;
        }
        size_t prependableBytes()const
        {
            return readIndex_;
        }

        void reset()
        {
            readIndex_=KinitPrepend;
            writeIndex_=KinitPrepend;
        }

        std::string getalldataAsstring()//get all data as string
        {
            std::string str(readptr(),readableBytes());
            reset();
            return str;
        }

        //those functions to add data into buffer
        void append(const std::string& str)
        {
            append(str.data(),str.length());
        }

        void append(const char* data,size_t len)
        {
            ensureWritebytes(len);
            std::copy(data,data+len,writeptr());
            updateWriteIdx(len);
        }

        void append(const void* data,size_t len)
        {
            append(static_cast<const char*>(data),len);
        }

        void ensureWritebytes(size_t len)
        {
            if(writableBytes()<len)
            {
                makespace(len);
            }
            assert(writableBytes()>=len);
        }

        const char* readptr() const
        {
            return begin()+readIndex_;
        }

        char* writeptr()
        {
            return begin()+writeIndex_;
        }
        const char* writeptr()const
        {
            return begin()+writeIndex_;
        }
        //those function used to update index
        void updateWriteIdx(size_t len)///update the writeIndex_;
        {
            writeIndex_+=len;
        }

        void updateReadIdx(size_t len)
        {
            assert(len<=readableBytes());
            readIndex_+=len;
        }

        void updateReadbyptr(const char* end)
        {
            assert(readptr()<=end);
            assert(end<=writeptr());
            updateReadIdx(end-readptr());
        }

        void prepend(const void* data,size_t len)//append at head and update readIndex_
        {
            assert(len<=prependableBytes());
            readIndex_-=len;
            const char* data_=static_cast<const char*>(data);
            std::copy(data_,data_+len,begin()+readIndex_);
        }

        void shrink(size_t reserve)//why don't change the writeIndex_ and readIndxe_??????
        {
            std::vector<char> buf(KinitPrepend+readableBytes()+reserve);
            std::copy(readptr(),readptr()+readableBytes(),buf.begin()+KinitPrepend);
            buf.swap(buffer_);
        }

        ssize_t readFd(int fd,int* savedErr);//??????

    private:
        char* begin(){return &*buffer_.begin();}
        const char* begin()const {return &*buffer_.begin();}
        void makespace(size_t len)
        {
            if(KinitPrepend+len>writableBytes()+prependableBytes())
            {
                buffer_.resize(writeIndex_+len);//can fully store data(len) at end 
            }
            else//move data to the begin to make enough space at end to contain the len data
            {
                assert(KinitPrepend<readIndex_);
                size_t readable=readableBytes();
                std::copy(begin()+readIndex_,begin()+writeIndex_,begin()+KinitPrepend);
                readIndex_=KinitPrepend;
                writeIndex_=readIndex_+readable;
                assert(readable==readableBytes());
            }
        }

        /* data */
        std::vector<char> buffer_;
        size_t readIndex_;
        size_t writeIndex_;
    };

}

#endif