/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-05-18 16:39:04
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-07-05 16:18:30
 * @FilePath: /MyServer/net/EventLoop.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "net/EventLoop.h"
#include "net/Epoller.h"
#include "net/Channel.h"

#include <unistd.h>
#include <sys/eventfd.h>
#include "EventLoop.h"

namespace net
{
    const int kEpollTimeMs = 10000;
    int createEventfd()
    {
        //返回了一个文件描述符，可以对其读写
        int evfd=::eventfd(0,EFD_NONBLOCK|EFD_CLOEXEC);
        return evfd;
    }

    EventLoop::EventLoop()
    :m_quit(false),
    m_iteration(0),
    m_ptrEpoller(new Epoller(this)),
    m_iThreadId(getTidOfThread()),
    m_wakeUpFd(createEventfd()),
    m_wakeUpChannel(new Channel(this,m_wakeUpFd,sockaddr_in())),
    m_currentActiveChannel(nullptr),
    m_ptrTimerQueue(new TimerQueue(this))
    {
        //printf("eventloop thread tid:%d\n",m_iThreadId);
        m_wakeUpChannel->setReadCallback(std::bind(&EventLoop::handleRead,this,std::placeholders::_1));
        m_wakeUpChannel->enableReading();
    }

    EventLoop::~EventLoop()
    {
        m_wakeUpChannel->disableAll();
        m_wakeUpChannel->remove();
        ::close(m_wakeUpFd);
    }
    void EventLoop::loop()
    {
        assertInLoopThread();
        //开启事件循环
        m_bLooping=true;
        m_quit=false;

        while(!m_quit)
        {
            m_listActiveChannel.clear();
            //等待epoll返回一个epoll中激活的channel的列表
            m_ptrEpoller->poll(kEpollTimeMs,&m_listActiveChannel);
            ++m_iteration;

            m_bEventHandling=true;
            //依次处理每一个激活的channel
            if(m_listActiveChannel.size()>0)
            {
                //printf("epoll jihuo,tid:%d\n",getTidOfThread());
            }
            for(auto& channel:m_listActiveChannel)
            {
                m_currentActiveChannel=channel;
                m_currentActiveChannel->handleEvent();
            }
            m_currentActiveChannel=nullptr;
            m_bEventHandling=false;
            doPendingFunctors();
        }
        m_bLooping=false;
    }
    void EventLoop::quit()
    {
        m_quit=true;
        if(!isInLoopThread())
        {
            wakeUp();
        }
    }
    void EventLoop::runInLoop(Functor cb)
    {
        if(isInLoopThread())
        {
            //printf("run in loop thread,tid:%d\n",getTidOfThread());
            cb();
        }
        else
        {
            queueInLoop(std::move(cb));
        }
    }
    void EventLoop::queueInLoop(Functor cb)
    {
        {
            std::lock_guard<std::mutex> lck(m_mutex);
            m_vecPendingFunctor.push_back(cb);
        }
        if(!isInLoopThread()||m_bCallingPendingFunctor)
        {
            wakeUp();
        }
    }
    void EventLoop::abortNotInLoopThread()
    {
        printf("event run not in thread,created tid:%d,current tid:%d\n",m_iThreadId,getTidOfThread());
    }
    size_t EventLoop::queueSize() const
    {
        return size_t();
    }
    void EventLoop::wakeUp()
    {
        uint64_t one=1;
        //向唤醒用描述符写入，使其可读。
        ssize_t n=::write(m_wakeUpFd,&one,sizeof(one));
    }
    
    void EventLoop::updateChannel(Channel *channel)
    {
        assertInLoopThread();
        m_ptrEpoller->updateChannel(channel);
    }
    void EventLoop::removeChannel(Channel *channel)
    {
        assertInLoopThread();
        m_ptrEpoller->removeChannel(channel);
    }
    bool EventLoop::hasChannel(Channel *channel)
    {
        assertInLoopThread();
        return m_ptrEpoller->hasChannel(channel);
    }
    Timer *EventLoop::runAt(TimeStamp time, TimerCallback cb)
    {
        return m_ptrTimerQueue->addTimer(std::move(cb),time,0.0);
    }

    Timer *EventLoop::runAfter(double delay, TimerCallback cb)
    {
        auto time=addTime(TimeStamp::now(),delay);
        return runAt(time,cb);
    }

    Timer *EventLoop::runEvery(double interval, TimerCallback cb)
    {
        auto time=addTime(TimeStamp::now(),interval);
        return m_ptrTimerQueue->addTimer(std::move(cb),time,interval);
    }

    void EventLoop::cancel(Timer *timer)
    {
        m_ptrTimerQueue->cancel(timer);
    }

    void EventLoop::handleRead(const TcpConnectionPtr& coon)
    {
        uint64_t one=1;
        ssize_t n=read(m_wakeUpChannel->fd(),&one,sizeof(one));
        //读取一下，不然一直处于可读状态
        if (n != sizeof(uint64_t))
        {
            printf("byte error,n=:%d,sizeof:%d\n",n,sizeof(uint64_t));
        }
    }
    void EventLoop::doPendingFunctors()
    {
        std::vector<Functor> functors;
        m_bCallingPendingFunctor=true;
        {
            std::lock_guard<std::mutex> lck(m_mutex);
            functors.swap(m_vecPendingFunctor);
        }
        for(auto& functor:functors)
        {
            functor();
        }
        m_bCallingPendingFunctor=false;
    }
}