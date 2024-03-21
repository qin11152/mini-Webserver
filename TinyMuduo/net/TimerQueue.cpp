/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-02 15:50:33
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-12 11:44:34
 * @FilePath: /MyServer/net/TimerQueue.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#include "net/Timer.h"
#include "net/EventLoop.h"
#include "net/TimerQueue.h"

#include <unistd.h>
#include <sys/timerfd.h>

namespace net
{
    int createTimerfd()
    {
        //调用系统接口创建一个定时器（文件描述符）
        int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                        TFD_NONBLOCK | TFD_CLOEXEC);
        return timerfd;
    }
    struct timespec howMuchTimeFromNow(TimeStamp when)
    {
        int64_t microseconds = when.getMsFromEpoch()
                            - TimeStamp::now().getMsFromEpoch();
        if (microseconds < 100)
        {
            microseconds = 100;
        }
        struct timespec ts;
        ts.tv_sec = static_cast<time_t>(
            microseconds / TimeStamp::kmsPerSecond);
        ts.tv_nsec = static_cast<long>(
            (microseconds % TimeStamp::kmsPerSecond) * 1000);
        return ts;
    }

    /// @brief 读取一下系统生成的定时器描述符，否则一直处于可读状态
    /// @param timerfd 
    /// @param now 
    void readTimerfd(int timerfd, TimeStamp now)
    {
        uint64_t howmany;
        //读取出来的大小应该是8字节
        ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    }

    /// @brief 重置超时时间
    /// @param timerfd 系统返回的定时器
    /// @param expiration 下次超时时间
    void resetTimerfd(int timerfd, TimeStamp expiration)
    {
        // wake up loop by timerfd_settime()
        //newvalue中第一个参数代表多久唤醒，第二个参数如果不为0就在第一次唤醒后没间隔第二个秒数唤醒，为0就是唤醒一次
        struct itimerspec newValue;
        struct itimerspec oldValue;
        memset(&newValue, 0, sizeof newValue);
        memset(&oldValue, 0, sizeof oldValue);
        //计算下多久唤醒，只唤醒一次
        newValue.it_value = howMuchTimeFromNow(expiration);
        //给定时器设置进去
        int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    }

    TimerQueue::TimerQueue(EventLoop * loop)
    :m_ptrLoop(loop),
    m_timerFd(createTimerfd()),
    m_timeChannel(new Channel(m_ptrLoop,m_timerFd,sockaddr_in())),
    m_timerList(),
    m_bCallingExpiredTimer(false)
    {
        m_timeChannel->setReadCallback(std::bind(&TimerQueue::handleRead,this,std::placeholders::_1));
        m_timeChannel->enableReading();
    }

    Timer* TimerQueue::addTimer(TimerCallback cb, TimeStamp whenExpired, double interval)
    {
        //根据传输的参数new出来一个定时器
        Timer* timer=new Timer(std::move(cb),whenExpired,interval);
        m_ptrLoop->runInLoop(std::bind(&TimerQueue::addTimerInLoop,this,timer));
        return timer;
    }

    void TimerQueue::cancel(Timer* timer)
    {
        m_ptrLoop->runInLoop(std::bind(&TimerQueue::cancelTimerInLoop,this,timer));
    }

    void net::TimerQueue::addTimerInLoop(Timer *timer)
    {
        bool result=insert(timer);
        //添加完看一下是否超时时间变化了，如果变了，说明当前的最小最靠前
        if(result)
        {
            //获取下这次定时器的超时时间，更新下
            resetTimerfd(m_timerFd,timer->expiration());
        }
    }

    void TimerQueue::cancelTimerInLoop(Timer* timer)
    {
        ActiveTimer acTimer(timer,timer->getSequence());
        auto iter=m_activeSets.find(acTimer);
        
        //如果找到了这个timer
        if(iter!=m_activeSets.end())
        {
            //从两个列表中都移除
            size_t n=m_timerList.erase(Entry(iter->first->expiration(),iter->first));
            delete iter->first;
            m_activeSets.erase(iter);
        }
        //如果正在执行任务，可能因为到时被移除而找不到，就添加到待删除列表
        else if(m_bCallingExpiredTimer)
        {
            m_cancelTime.insert(acTimer);
        }
    }

    void TimerQueue::handleRead(const TcpConnectionPtr& coon)
    {
        //获取当时时间
        TimeStamp now(TimeStamp::now());
        readTimerfd(m_timerFd,now);
        
        //获取下这个超时时间下所有到期的定时器
        auto expiredVec=getExpired(now);

        m_bCallingExpiredTimer=true;
        m_cancelTime.clear();

        //对于已经超时的定时器，执行他们的回调函数
        for(auto& item:expiredVec)
        {
            item.second->run();
        }

        m_bCallingExpiredTimer=false;
        reset(expiredVec,now);
    }

    std::vector<Entry> TimerQueue::getExpired(TimeStamp now)
    {
        std::vector<Entry> expiredVec;
        //根据此时的时间，生成一个entry对象，第二个值是一个很大的数，指针不会大于它
        //如果不是一个很大的，先比较第一个，再比较第二个，可能就会略过已经到时的定时器
        Entry entry(now,reinterpret_cast<Timer*>(UINTPTR_MAX));
        //找到第一个大于等于entry的，其实不会等于，UINTPTR_MAX太大了
        auto iter=m_timerList.lower_bound(entry);
        //把这些都插入到返回值vec中
        std::copy(m_timerList.begin(),iter,back_inserter(expiredVec));
        //从timestamp，timer的set中移除
        m_timerList.erase(m_timerList.begin(),iter);
        for (const Entry& it : expiredVec)
        {
            //从timer，sequence的set中移除
            ActiveTimer timer(it.second, it.second->getSequence());
            size_t n = m_activeSets.erase(timer);
        }
        return expiredVec;
    }

    void TimerQueue::reset(const std::vector<Entry> &expired, TimeStamp now)
    {
        TimeStamp nextExpired;
        for(auto item:expired)
        {
            //看一下是否能重复和是否被移除了
            ActiveTimer timer(item.second,item.second->getSequence());
            if(item.second->isRepeat()&&!m_cancelTime.count(timer))
            {
                //满足条件就插回去
                item.second->restart(now);
                insert(item.second);
            }
            else
            {
                delete item.second;
            }
        }
        if(!m_timerList.empty())
        {
            nextExpired=m_timerList.begin()->second->expiration();
        }

        if(nextExpired.isvalid())
        {
            resetTimerfd(m_timerFd,nextExpired);
        }
    }

    bool TimerQueue::insert(Timer *timer)
    {
        bool reuslt=false;
        TimeStamp when=timer->expiration();
        TimerList::iterator iter=m_timerList.begin();
        //如果当前timer的列表为空，或者超时时间小于set中的第一个(set是以超时时间从小到大排列的)
        //就代表需要改变超时时间了,要变小了
        if(iter==m_timerList.end()||when<iter->first)
        {
            reuslt=true;
        }
        //两个set中都插入这个新的定时器
        m_timerList.insert(Entry(when,timer));
        m_activeSets.insert(ActiveTimer(timer,timer->getSequence()));
        
        return reuslt;
    }
}
