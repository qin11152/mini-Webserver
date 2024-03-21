#include "Epoller.h"
#include "Channel.h"

#include <poll.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>

namespace net
{
    Epoller::Epoller(EventLoop *loop)
        :m_eventLoop(loop),
        m_epollFd(::epoll_create1(EPOLL_CLOEXEC)),
        m_listEvents(kInitEventListSize)
    {
        //printf("epoller thread id:%d\n",getTidOfThread());
    }

    Epoller::~Epoller()
    {
    }

    void Epoller::poll(int timeoutMs, ChannelList *activeChannels)
    {
        //等待epoll返回就绪的事件
        int numreadyEvents=epoll_wait(m_epollFd,&*m_listEvents.begin(),m_listEvents.size(),timeoutMs);
        auto error=errno;
        
        //如果返回的大于0
        if(0<numreadyEvents)
        {
            fillActiveChannels(numreadyEvents,activeChannels);
            if(static_cast<size_t>(numreadyEvents)==m_listEvents.size())
            {
                //手动扩容
                m_listEvents.reserve(m_listEvents.size()*2);
            }
        }
        else if(0==numreadyEvents)
        {
            //printf("epoll timeout and noting happen\n");
        }
        else
        {
            if (error != EINTR)
            {
                printf("epoll_wait error:%d\n",error);
            }
        }
    }

    void Epoller::updateChannel(Channel *channel)
    {
        m_eventLoop->assertInLoopThread();
        //获取channel的index
        const int index=channel->index();

        //如果是新new或者deleted的状态，就是要添加到epoll中
        if(kNew==index||kDeleted==index)
        {
            //获取文件描述符
            int fd=channel->fd();
            //新增的话就保存在map中，deleted过的map里已经有了
            if(kNew==index)
            {
                //assert(m_mapChannels.count(channel->fd)==0);
                m_mapChannels[fd]=channel;
            }
            //状态设置为added
            channel->setIndex(kAdded);
            update(EPOLL_CTL_ADD,channel);
        }
        else
        {
            //如果是已经添加的channel
            //int fd=channel->fd();
            //没有任何关注事件
            if(channel->isNoneEvent())
            {
                //从epoll中移除,但没有从map中移除
                update(EPOLL_CTL_DEL,channel);
                channel->setIndex(kDeleted);
            }
            else
            {
                //修改channel在epoll中注册的关注事件，事件是channel->events()接口获取的
                update(EPOLL_CTL_MOD,channel);
            }
        }
    }

    void Epoller::removeChannel(Channel *channel)
    {
        printf("poll remove channel\n");
        m_eventLoop->assertInLoopThread();
        int fd=channel->fd();
        int index=channel->index();
        size_t n=m_mapChannels.erase(fd);
        //从epoll中移除，如果没有added过就不用移除了
        if(kAdded==index)
        {
            update(EPOLL_CTL_DEL,channel);
        }
        //更新channel状态未knew
        channel->setIndex(kNew);
    }

    bool Epoller::hasChannel(Channel *channel)
    {
        m_eventLoop->assertInLoopThread();
        auto iter=m_mapChannels.find(channel->fd());
        return iter!=m_mapChannels.end()&&iter->second==channel;
    }

    void Epoller::fillActiveChannels(int numEvents, ChannelList *activeChannels)
    {
        for(int i=0;i<numEvents;++i)
        {
            //获取到channel指针
            Channel* channel=static_cast<Channel*>(m_listEvents[i].data.ptr);
            //根据channel指针获取到需要的信息，如文件描述符等
            int fd=channel->fd();
            auto iter=m_mapChannels.find(fd);
            //设置此当前的事件
            channel->setCurrentEvents(m_listEvents[i].events);
            //push到active的channel列表中
            activeChannels->push_back(channel);
        }
    }

    void Epoller::update(int operation, Channel *channel)
    {
        struct epoll_event event;
        memset(&event,0,sizeof(event));
        event.events=channel->events();
        event.data.ptr=channel;
        int fd=channel->fd();
        if(::epoll_ctl(m_epollFd,operation,fd,&event)<0)
        {
            printf("epoll ctl error,operation:%d\n",operation);
        }
    }
}