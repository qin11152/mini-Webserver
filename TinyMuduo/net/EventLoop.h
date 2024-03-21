/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-05-18 16:39:04
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-12 11:45:54
 * @FilePath: /MyServer/net/EventLoop.h
 * @Description: 事件循环类，在这里while循环调用epoll并对相应的读写事件进行处理
 */
#pragma once
#include "net/TimerQueue.h"
#include "net/Callbacks.h"

#include <atomic>
#include <vector>
#include <mutex>
#include <sys/syscall.h>

namespace net
{
    class Channel;
    class Epoller;
    using ChannelList=std::vector<Channel*>;
    
    inline pid_t getTidOfThread()
    {
        return static_cast<pid_t>(::syscall(SYS_gettid));
    }

    class EventLoop
    {
    public:
        explicit EventLoop();
        ~EventLoop();

        void loop();

        void quit();

        int64_t iteration()const {return m_iteration;}

        void runInLoop(Functor cb);

        void queueInLoop(Functor cb);

        void assertInLoopThread()
        {
            if(!isInLoopThread())
            {
                abortNotInLoopThread();
            }
        }
        bool isInLoopThread()const{return m_iThreadId==getTidOfThread();}
        void abortNotInLoopThread();

        size_t queueSize()const;

        void wakeUp();
        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);
        bool hasChannel(Channel* channel);

        bool eventHanding()const {return m_bEventHandling;}

        //定时器相关

        Timer* runAt(TimeStamp time,TimerCallback cb);
        Timer* runAfter(double delay,TimerCallback cb);
        Timer* runEvery(double interval,TimerCallback cb);
        void cancel(Timer* timer);

    private:
        void handleRead(const TcpConnectionPtr& coon);
        void doPendingFunctors();

    private:
        //标志是否正在进行事件循环
        bool m_bLooping{false};
        //退出循环标志位
        std::atomic<bool> m_quit;
        //是否正在进行事件处理
        bool m_bEventHandling{false};
        //调用等待中的事件回调
        bool m_bCallingPendingFunctor{false};
        int64_t m_iteration;
        //这个eventloop对应的线程id
        const pid_t m_iThreadId;
        //epoll对象
        std::unique_ptr<Epoller> m_ptrEpoller;
        //唤醒用文件描述符，自己主动唤醒epoll，非epoll被激活
        int m_wakeUpFd;
        //唤醒描述符对应的channel
        std::shared_ptr<Channel> m_wakeUpChannel;
        //计时器队列
        std::unique_ptr<TimerQueue> m_ptrTimerQueue;
        TimeStamp m_pollerRerturnTime;

        //epoll中激活的channel列表
        ChannelList m_listActiveChannel;
        //当前激活的一个channel
        Channel* m_currentActiveChannel;

        mutable std::mutex m_mutex;
        //等待执行的回调函数列表
        std::vector<Functor> m_vecPendingFunctor;
    };
}