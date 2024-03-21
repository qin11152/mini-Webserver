/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-14 14:33:12
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-15 10:28:15
 * @FilePath: /MyServer/TinyMuduo/base/Buffer.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "Buffer.h"
#include <sys/uio.h> 

namespace base
{
    ssize_t Buffer::readFd(int fd, int * savedError)
    {
        char extraBuffer[65536]{0};
        struct iovec vec[2];
        //可写的长度
        const size_t writable = writableBytes();
        //首先是自己的buffer，这是第一选择
        vec[0].iov_base = begin()+m_iWriterIndex;
        vec[0].iov_len = writable;
        //第二个是extrabuffer
        vec[1].iov_base = extraBuffer;
        vec[1].iov_len = sizeof(extraBuffer);
        // when there is enough space in this buffer, don't read into extrabuf.
        // when extrabuf is used, we read 128k-1 bytes at most.
        //buffer可写的长度小于extrabuffer的时候读取到extrabuffer
        const int iovcnt = (writable < sizeof(extraBuffer)) ? 2 : 1;

        //readv中第二个参数是一个列表（vector），第三个参数是列表的长度
        //如果列表大于1先往buffer1中写，写满了再去buffer中写
        const ssize_t n = readv(fd, vec, iovcnt);
        if (n < 0)
        {
            *savedError = errno;
        }
        //读取到的字节小于可写的长度
        else if (static_cast<size_t>(n) <= writable)
        {
            m_iWriterIndex += n;
        }
        //读取的长度大于可写的长度，当前写的位置在尾部，然后把在extrabuffer中的添加到buffer中
        else
        {
            m_iWriterIndex = m_buffer.size();
            append(extraBuffer, n - writable);
        }

        return n;
    }
}


