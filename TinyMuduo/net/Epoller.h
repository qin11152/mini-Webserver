/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-05-18 16:39:04
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-09 14:35:16
 * @FilePath: /MyServer/net/Epoller.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include "net/EventLoop.h"
#include <vector>
#include <map>

struct epoll_event;

namespace
{
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}

namespace net
{
    class Channel;

    using EventList=std::vector<struct epoll_event>;
    using ChannelList=std::vector<Channel*>;
    using ChannelMap=std::map<int, Channel*>;

    class Epoller
    {
    public:
        explicit Epoller(EventLoop* loop);
        ~Epoller();

        /// @brief 这里就是调用epollwait，用来阻塞等待返回所有就绪事件
        /// @param activeChannels ：就绪的channel，epoll中添加的event中的ptr就是channel
        /// @param timeoutMs：epoll超时时间
        void poll(int timeoutMs, ChannelList* activeChannels);
        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);
        bool hasChannel(Channel* channel);

    private:
        static constexpr int kInitEventListSize=16;
        
        /// @brief 根据epoll获取到的活动列表，设置每个激活的channel的当前事件，并将激活的cahnnel存在列表中
        /// @param numEvents ：激活的数量
        /// @param activeChannels ：被激活的channel
        void fillActiveChannels(int numEvents, ChannelList* activeChannels);

        /// @brief 用来想epoll中添加，删除或修改关注事件的借口
        /// @param operation 要进行的操作
        /// @param channel 要操作的channel对象
        void update(int operation,Channel* channel);

    private:
        int m_epollFd;
        ChannelMap m_mapChannels;
        EventLoop* m_eventLoop;
        EventList m_listEvents;
    };
}