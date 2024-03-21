/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-06 16:18:57
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-09 14:30:37
 * @FilePath: /MyServer/net/EventLoopThreadPool.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "EventLoopThreadPool.h"

namespace net
{
    EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop)
    :m_ptrLoop(baseLoop),
    m_iNumbers(0),
    m_iNext(0)
    {
    }
    EventLoopThreadPool::~EventLoopThreadPool()
    {
    }
    void EventLoopThreadPool::start(const ThreadInitCallback &cb)
    {
        m_bStarted=true;

        for(int i=0;i<m_iNumbers;++i)
        {
            EventLoopThread* t=new EventLoopThread(cb);
            m_vecEventloopThreads.push_back(std::unique_ptr<EventLoopThread>(t));
            m_vecEventLoops.push_back(t->startEventLoop());
        }

        if(m_iNumbers==0&&cb)
        {
            cb(m_ptrLoop);
        }
    }
    EventLoop *EventLoopThreadPool::getNextLoop()
    {
        m_ptrLoop->assertInLoopThread();
        EventLoop* loop=m_ptrLoop;
        if(!m_vecEventLoops.empty())
        {
            loop=m_vecEventLoops[m_iNext];
            ++m_iNext;
            if(m_iNext>=m_vecEventLoops.size())
            {
                m_iNext=0;
            }
        }
        return loop;
    }
    std::vector<EventLoop *> EventLoopThreadPool::getAllLoop() const
    {
        m_ptrLoop->assertInLoopThread();
        if(m_vecEventLoops.empty())
        {
            return std::vector<EventLoop*>(1,m_ptrLoop);
        }
        else
        {
            return m_vecEventLoops;
        }
    }
}