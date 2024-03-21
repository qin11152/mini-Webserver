/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-02 14:00:38
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-02 15:17:55
 * @FilePath: /MyServer/net/Timer.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#include "Timer.h"

namespace net
{
    std::atomic<int> Timer::m_iNumCreated;

    Timer::Timer(const TimerCallback &callback, const TimeStamp &whenExpired, double interval):
        m_callBack(callback),
        m_expiration(whenExpired),
        m_dInternal(interval),
        m_bRepeat(interval>0.0),
        m_iSequence(++m_iNumCreated)
    {
    }

    void Timer::run() const
    {
        m_callBack();
    }

    void Timer::restart(const TimeStamp &now)
    {
        //可重复的定时器才能restart
        if(m_bRepeat)
        {
            m_expiration=addTime(now,m_dInternal);
        }
        //不可重复的给他一个无效的timestamp
        else
        {
            m_expiration=TimeStamp::invalid();
        }
    }
}