#include "net/Buffer.h"

#include <sys/uio.h>
#include <errno.h>
#include <unistd.h>
// 从fd上读取数据 Poller工作在LT模式 Buffer缓冲区是有大小的，但是从fd上读数据的时候，却不知道tcp数据最终的大小
ssize_t Buffer::readFd(int fd, int* savedErrno)
{
    char extrabuf[65536] = {0}; // 栈上的内存空间 64k
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    
    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *savedErrno = errno;
    }
    else if (n <= writable) //Buffer的可写缓冲区已经够存储读出来的数据
    {
        writerIndex_ += n;
    }
    else // extrabuf 里面也写入了数据
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable); // writerIndex_开始写 n - writebale大小的数据
    }

    return n;
}

ssize_t Buffer::writeFd(int fd, int* saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}