/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-06 15:49:21
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-06 16:18:00
 * @FilePath: /MyServer/net/EventLoopThread.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include <boost/noncopyable.hpp>

#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>

namespace net
{
    class EventLoop;
    using ThreadInitCallback=std::function<void(EventLoop*)>;
    class EventLoopThread : public boost::noncopyable
    {
    public:
        explicit EventLoopThread(const ThreadInitCallback& cb);
        /// @brief loop指针退出循环，线程join
        ~EventLoopThread();

        /// @brief 子线程中创建loop并开始循环，返回创建的指针
        /// @return 
        EventLoop* startEventLoop();

    private:
        /// @brief 创建一个loop对象，创建成功后会赋值给成员变量，并开始loop
        void threadFunc();

        bool m_bExited;
        //事件循环类
        EventLoop* m_ptrEventLoop;
        std::mutex m_mutex;
        std::condition_variable m_conVariable;
        //这个事件循环在的线程
        std::thread m_thread;
        //线程的事件回调，在构造线程前调用
        ThreadInitCallback m_threadInitCB;
    };
}
