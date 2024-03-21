/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-02 15:50:20
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-12 11:44:25
 * @FilePath: /MyServer/net/TimerQueue.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include <set>
#include <vector>

#include "base/TimeStamp.h"
#include "net/Channel.h"

#include <functional>

namespace net
{
    class EventLoop;
    class Timer;

    /// @brief pair（超时时间timestamp和timer）
    using Entry=std::pair<TimeStamp,Timer*>;
    //里面是键值对（key：TimeStamp，value：Timer*）
    using TimerList=std::set<Entry>;
    //pair（timer和序列号）
    using ActiveTimer=std::pair<Timer*,uint64_t>;
    //里面也是键值对（key：Timer*，value：uint64），后边的是timer的唯一标识
    using ActiveTimerSet=std::set<ActiveTimer>;

    class TimerQueue
    {
    public:
        using TimerCallback=std::function<void()>;
        explicit TimerQueue(EventLoop* loop);

        /// @brief 调用addtimerinloop向定时器队列中增加一个定时器
        /// @param cb 定时器所对应的回调函数
        /// @param whenExpired 超时时间
        /// @param interval 间隔
        /// @return 
        Timer* addTimer(TimerCallback cb,TimeStamp whenExpired,double interval);

        /// @brief 取消一个定时器
        /// @param timer 定时器
        void cancel(Timer* timer);

    private:
        /// @brief 添加定时器
        /// @param timer 要添加的那个定时器
        void addTimerInLoop(Timer* timer);
        void cancelTimerInLoop(Timer* timer);

        /// @brief 处理定时器对应描述符可读事件
        void handleRead(const TcpConnectionPtr& coon);
        /// @brief 获取这个时刻下所有的超时的定时器对象
        /// @param now 对应的时刻
        /// @return 
        std::vector<Entry> getExpired(TimeStamp now);
        /// @brief 重置那些有重复属性的定时器
        /// @param expired 已经超时的定时器
        /// @param now 当前时间
        void reset(const std::vector<Entry>& expired,TimeStamp now);

        /// @brief 添加定时器实际的接口，添加的时候还要判断下新插入的是否会影响当前到时时间（小于当前的就更新）
        /// @param timer 
        /// @return 
        bool insert(Timer* timer);
    private:

        EventLoop* m_ptrLoop{nullptr};
        //调用系统接口创建的一个定时器，是一个文件描述符，超时时变为可读，可被epoll监听
        const int m_timerFd;
        std::shared_ptr<Channel> m_timeChannel;
        //定时器timer列表，存储着超时时间和对应的定时器timer
        TimerList m_timerList;
        //存储着激活的timer和其对应的序列号
        ActiveTimerSet m_activeSets;
        bool m_bCallingExpiredTimer;
        //等待取消的timer和其序列号
        ActiveTimerSet m_cancelTime;
    };
}
