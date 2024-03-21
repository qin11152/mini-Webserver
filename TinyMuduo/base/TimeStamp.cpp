/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-02 11:09:33
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-02 18:23:35
 * @FilePath: /MyServer/base/TimeStamp.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "TimeStamp.h"
#include <inttypes.h>
#include <sys/time.h>

std::string TimeStamp::toString() const
{
    char buf[32]{0};
    uint64_t seconds=m_msFromEpoch/kmsPerSecond;
    uint64_t mSeconds=m_msFromEpoch%kmsPerSecond;
    //PRIu64是为了区分32（llu）位和64（lu）位，
    snprintf(buf,sizeof(buf),"%" PRIu64 ".%06" PRIu64 "",seconds,mSeconds);
    return buf;
}

std::string TimeStamp::toFromatString(bool showMicroseconds) const
{
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(m_msFromEpoch / kmsPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    if (showMicroseconds)
    {
    int microseconds = static_cast<int>(m_msFromEpoch % kmsPerSecond);
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
                microseconds);
    }
    else
    {
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}

TimeStamp TimeStamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return TimeStamp(seconds * kmsPerSecond + tv.tv_usec);
}
