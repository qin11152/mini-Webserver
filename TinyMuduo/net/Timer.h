/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-02 14:00:34
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-05 16:25:15
 * @FilePath: /MyServer/net/Timer.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include "base/TimeStamp.h"
#include "net/Callbacks.h"
#include <atomic>

namespace net
{
    class Timer
    {
    public:
        Timer(const TimerCallback& callback,const TimeStamp& whenExpired,double interval);

        /// @brief 执行回调函数
        void run()const;

        TimeStamp expiration()const {return m_expiration;}
        bool isRepeat()const{return m_bRepeat;}
        uint64_t getSequence()const{return m_iSequence;}
        
        /// @brief 重置定时器，根据是否repeat来操作，repeat的才重置
        /// @param now 传进来的是当前的一个时间，下次超时时间就是从当前+internal
        void restart(const TimeStamp& now);

        static uint64_t getNumCreated(){return m_iNumCreated;}

    private:
        //定时器到时回调函数
        const TimerCallback m_callBack;
        //定时器到时时间，由timestamp封装
        TimeStamp m_expiration;
        //设置的定时间隔，如果不重复的话就为0
        const double m_dInternal;
        //定时器是否重复
        const bool m_bRepeat;
        //定时器的id，唯一标识
        const uint64_t m_iSequence;


        //用来记录已经产生了多少个timer
        static std::atomic<int> m_iNumCreated;
    };
}
