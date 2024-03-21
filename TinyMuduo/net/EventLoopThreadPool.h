/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-06 16:18:36
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-08 10:39:48
 * @FilePath: /MyServer/net/EventLoopThreadPool.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include "EventLoop.h"
#include "EventLoopThread.h"

#include <boost/noncopyable.hpp>

#include <memory>
#include <vector>
#include <functional>

namespace net
{
    using ThreadInitCallback=std::function<void(EventLoop*)>;
    class EventLoopThreadPool:public boost::noncopyable
    {
    public:
        explicit EventLoopThreadPool(EventLoop* baseLoop);
        ~EventLoopThreadPool();
        void setThreadNums(int threadNums){m_iNumbers=threadNums;}
        /// @brief 根据线程池的数量，开启对应数量的线程，每个线程都是一个loop循环
        /// @param cb 
        void start(const ThreadInitCallback& cb);

        /// @brief 获取下一个loop，如果这个线程池数量为0,就返回baseloop
        /// @return 
        EventLoop* getNextLoop();

        std::vector<EventLoop*> getAllLoop()const;

        bool isStarted()const{return m_bStarted;}

    private:
        //eventloop
        EventLoop* m_ptrLoop;
        bool m_bStarted{false};
        int m_iNext;
        int m_iNumbers;
        std::vector<std::unique_ptr<EventLoopThread>> m_vecEventloopThreads;
        std::vector<EventLoop*> m_vecEventLoops;
    };
}