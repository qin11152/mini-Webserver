/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-02 11:09:26
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-14 14:03:43
 * @FilePath: /MyServer/base/TimeStamp.h
 * @Description: 时间戳类，用于获取时间，时间的一些比较，加减计算
 */

#pragma once

#include "boost/operators.hpp"

#include <stdint.h>
#include <string.h>
#include <string>

/// @brief 对时间的封装，可以按格式返回记录的时刻
class TimeStamp : public boost::equality_comparable<TimeStamp>
{
public:

    TimeStamp():
    m_msFromEpoch(0)
    {
    }
    explicit TimeStamp(uint64_t msFromEpoch):
    m_msFromEpoch(msFromEpoch)
    {
    }

    bool operator==(const TimeStamp& r)
    {
        return m_msFromEpoch==r.getMsFromEpoch();
    }
    friend bool operator<(const TimeStamp& l,const TimeStamp& r);

    /// @brief 将时间戳内部的毫秒数转为字符串形式
    /// @return 返回的字符串
    std::string toString()const;

    /// @brief 以标准时间格式返回当前时间戳对应的时间
    /// @return 
    std::string toFromatString(bool showMicroseconds)const;

    /// @brief 获取当前保存的时间戳
    /// @return 
    uint64_t getMsFromEpoch()const{return m_msFromEpoch;}

    static TimeStamp now();

    /// @brief 返回一个非法的timestamp，调用默认构造函数，内部时间为0
    /// @return 
    static TimeStamp invalid()
    {
        return TimeStamp();
    }

    static TimeStamp fromUnixTime(time_t t)
    {
        return fromUnixTime(t, 0);
    }
    static TimeStamp fromUnixTime(time_t t,int ms)
    {
        return TimeStamp(static_cast<uint64_t>(t) * kmsPerSecond + ms);
    }
    //每秒有多少微秒
    static constexpr int kmsPerSecond=1000*1000;
    
    bool isvalid()const {return m_msFromEpoch>0;}

private:
    //微妙，从1900开始计算
    uint64_t m_msFromEpoch;
};

inline bool operator<(const TimeStamp& l,const TimeStamp& r)
{
    return l.getMsFromEpoch()<r.getMsFromEpoch();
}

inline bool operator==(const TimeStamp& l,const TimeStamp& r)
{
    return l.getMsFromEpoch()==r.getMsFromEpoch();
}

inline double timeDiff(TimeStamp& left,TimeStamp& r)
{
    //计算时间差
    uint64_t diff=left.getMsFromEpoch()-r.getMsFromEpoch();
    //转换成秒数
    return static_cast<double>(diff)/TimeStamp::kmsPerSecond;
}

inline TimeStamp addTime(const TimeStamp& time,double seconds)
{
    uint64_t delta=static_cast<uint64_t>(seconds)*TimeStamp::kmsPerSecond;
    return TimeStamp(time.getMsFromEpoch()+delta);
}