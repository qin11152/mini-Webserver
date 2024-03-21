/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-05-18 16:39:04
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-15 10:26:04
 * @FilePath: /MyServer/net/Channel.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "Channel.h"
#include "Epoller.h"
#include "EventLoop.h"

#include <sys/socket.h>
#include <sys/uio.h> 
#include <poll.h>

namespace net
{
    constexpr int Channel::kNoneEvent = 0;
    constexpr int Channel::kReadEvent = POLLIN | POLLPRI;
    constexpr int Channel::kWriteEvent = POLLOUT;

    Channel::Channel(EventLoop *loop, int fd,sockaddr_in addr)
      : m_eventLoop(loop),
        m_peerAddr(addr),
        m_fd(fd),
        m_events(0),
        m_currentEvent(0),
        m_index(-1)
    {
    }

    Channel::~Channel()
    {
        printf("channel destroy\n");
    }

    void Channel::handleEvent()
    {
        std::shared_ptr<void> guard;
        if (m_bTied)
        {
            guard = m_tie.lock();
            if (guard)
            {
            handleEventWithGuard();
            }
        }
        else
        {
            handleEventWithGuard();
        }
    }

    void Channel::handleRead()
    {
        m_eventLoop->assertInLoopThread();
        int savedError=0;
        ssize_t n=m_inputBuffer.readFd(m_fd,&savedError);
        if(n>0)
        {
            if (m_readCB) m_readCB(shared_from_this());
        }
        else if(0==n)
        {
            handleClose();
        }
        else
        {
            printf("handle read error\n");
        }
    }

    void Channel::handleClose()
    {
        m_state=kDisconnected;
        disableAll();
        printf("before close, thread id:%d\n",getTidOfThread());
        m_closeCB(shared_from_this());
    }

    void Channel::handleWrite()
    {
        m_eventLoop->assertInLoopThread();
        //如果在监听可写的状态
        if(isWriting())
        {
            ssize_t n=write(m_fd,m_outputBuffer.peek(),m_outputBuffer.readableBytes());
            if(n>0)
            {
                m_outputBuffer.retrieve(n);
                if(0==m_outputBuffer.readableBytes())
                {
                    disableWriting();
                    if(m_writeCB)
                    {
                        m_eventLoop->queueInLoop(std::bind(m_writeCB,shared_from_this()));
                    }
                    if(kDisconnected==m_state)
                    {
                        shutDownInLoop();
                    }
                }
            }
            else
            {
                printf("handle write failed\n");
            }
        }
        else
        {
            printf("handle write error\n");
        }
    }

    void Channel::connectionEstablished()
    {
        printf("call estibilshed,tid:%d\n",getTidOfThread());
        m_state=kConnected;
        m_bTied=true;
        enableReading();
        if(m_connectionCallback)
        {
            m_connectionCallback(shared_from_this());
        }
    }

    void Channel::connectionDestreyed()
    {
        if(kConnected == m_state)
        {
            m_state=kDisconnected;
            disableAll();
        }
        remove();
    }

    void Channel::send(const void *message, int len)
    {
        sendInLoop(message,len);
    }

    void Channel::send(const std::string &message)
    {
        if(kConnected == m_state)
        {
            if(m_eventLoop->isInLoopThread())
            {
                sendInLoop(message.c_str(),message.size());
            }
            else
            {
                //类成员函数指针
                void (Channel::*fp)(const std::string& message)=&Channel::sendInLoop;
                //如果直接bind的话不知道调用哪一个sendinloop
                m_eventLoop->runInLoop(std::bind(fp,this,message));
            }
        }
    }

    int Channel::readIntoBuffer(int& savedError)
    {
        return m_inputBuffer.readFd(m_fd,&savedError);
    }

    void Channel::tie(const std::shared_ptr<void> &obj)
    {
        m_tie=obj;
        m_bTied=true;
    }

    void Channel::remove()
    {
        m_bAddedToLoop=false;
        m_eventLoop->removeChannel(this);
    }

    void Channel::shutDownInLoop()
    {
        if(!isWriting())
        {
            ::shutdown(m_fd,SHUT_WR);
        }
    }

    void Channel::sendInLoop(const std::string &message)
    {
        sendInLoop(message.c_str(),message.length());
    }

    void Channel::sendInLoop(const void *message, size_t len)
    {
        m_eventLoop->assertInLoopThread();
        ssize_t nwrote=0;
        size_t remaining=len;
        bool faultError=false;
        if(kDisconnected==m_state)
        {
            printf("call send and connection is disconneted\n");
            return;
        }
        if(!isWriting()&&0==m_outputBuffer.readableBytes())
        {
            nwrote=::write(m_fd,message,len);
            remaining=len-nwrote;
            //如果这次直接就写完了，那就调用writecallback
            if(0==remaining&&m_writeCB)
            {
                printf("send in once time\n");
                m_eventLoop->queueInLoop(std::bind(m_writeCB,shared_from_this()));
            }
            else
            {
                nwrote=0;
                if(errno!=EWOULDBLOCK)
                {
                    if(errno!=EPIPE||errno!=ECONNRESET)
                    {
                        //到这里说明写除了错误
                        faultError=true;
                    }
                }
            }
        }
        //运行到这里肯定是没写完，有可能是错误，也有可能一次没全发送出去

        //一次没发送完的情况
        if(!faultError&&remaining>0)
        {
            m_outputBuffer.append(static_cast<const char*>(message)+nwrote,remaining);
            if(!isWriting())
            {
                //监测可写事件，下次从buffer读并write
                enableWriting();
            }
        }
    }

    void Channel::update()
    {
        m_bAddedToLoop=true;
        m_eventLoop->updateChannel(this);
    }

    void Channel::handleEventWithGuard()
    {
        m_bEventHandling = true;
        if ((m_events & POLLHUP) && !(m_events & POLLIN))
        {
            if (m_closeCB) m_closeCB(shared_from_this());
        }

        if (m_events & POLLNVAL)
        {
        }

        if (m_events & (POLLERR | POLLNVAL))
        {
            if (m_errorCB) m_errorCB();
        }
        if (m_events & (POLLIN | POLLPRI | POLLRDHUP))
        {
            //handleRead();
            if(m_readCB) m_readCB(shared_from_this());
        }
        if (m_events & POLLOUT)
        {
            if (m_writeCB) m_writeCB(shared_from_this());
        }
        m_bEventHandling = false;
    }
}
