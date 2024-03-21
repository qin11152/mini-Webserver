/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-06 15:49:29
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-09 17:30:33
 * @FilePath: /MyServer/net/EventLoopThread.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "net/EventLoopThread.h"
#include "net/EventLoop.h"
#include "EventLoopThread.h"

namespace net
{
    EventLoopThread::EventLoopThread(const ThreadInitCallback &cb)
    :m_ptrEventLoop(nullptr),
    m_bExited(false),
    m_mutex(),
    m_conVariable(),
    m_threadInitCB(cb)
    {
    }

    EventLoopThread::~EventLoopThread()
    {
        m_bExited=true;
        if(nullptr!=m_ptrEventLoop)
        {
            m_ptrEventLoop->quit();
            m_thread.join();
        }
    }

    EventLoop *net::EventLoopThread::startEventLoop()
    {
        std::thread t1(std::bind(&EventLoopThread::threadFunc,this));
        m_thread=std::move(t1);
        EventLoop* loop=nullptr;
        {
            std::unique_lock<std::mutex> lck(m_mutex);
            while(nullptr==m_ptrEventLoop)
            {
                m_conVariable.wait(lck);
            }
            loop=m_ptrEventLoop;
        }
        return loop;
    }
    void EventLoopThread::threadFunc()
    {
        EventLoop loop;
        if(m_threadInitCB)
        {
            m_threadInitCB(&loop);
        }
        {
            std::unique_lock<std::mutex> lck(m_mutex);
            m_ptrEventLoop=&loop;
            m_conVariable.notify_all();
        }

        m_ptrEventLoop->loop();
        std::unique_lock<std::mutex> lck(m_mutex);
        m_ptrEventLoop=nullptr;
    }
}