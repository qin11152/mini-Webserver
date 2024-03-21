/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-05-18 16:39:04
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-14 19:19:49
 * @FilePath: /MyServer/net/TcpServer.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "net/EventLoopThreadPool.h"

#include <algorithm>

namespace net
{
    TcpServer::TcpServer(EventLoop *loop, const sockaddr_in &addr, int port)
    :m_ptrEventLoop(loop),
    m_ptrEventLoopThreadPool(new EventLoopThreadPool(loop)),
    m_ptrAcceptor(new Acceptor(loop,port,addr))
    {
        m_ptrAcceptor->setNewConnectionCB(std::bind(&TcpServer::newConnection,this,
                                                            std::placeholders::_1,std::placeholders::_2));
        //loop->runAfter(2.0,[](){printf("timer1 timeout\n");});
        // loop->runEvery(6.0,[](){printf("timer2 timeout\n");});
    }
    TcpServer::~TcpServer()
    {
        for(auto& channel:m_vecConnection)
        {
            //channel.reset();
            auto loop=channel->getLoop();
            loop->runInLoop(std::bind(&Channel::connectionDestreyed,channel.get()));
        }
        m_vecConnection.clear();
    }
    void TcpServer::start()
    {
        m_ptrEventLoopThreadPool->setThreadNums(5);
        m_ptrEventLoopThreadPool->start(ThreadInitCallback());
        m_ptrEventLoop->runInLoop(std::bind(&Acceptor::listen,m_ptrAcceptor.get()));
    }
    void TcpServer::removeConnection(const std::shared_ptr<Channel> &coon)
    {
        printf("call remove and thread id:%d\n",getTidOfThread());
        m_ptrEventLoop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop,this,coon));
    }
    void TcpServer::removeConnectionInLoop(const std::shared_ptr<Channel> &coon)
    {
        printf("server remove channel\n");
        m_vecConnection.erase(std::find(m_vecConnection.begin(),m_vecConnection.end(), coon));
        coon->getLoop()->queueInLoop(std::bind(&Channel::connectionDestreyed,coon));
    }
    void TcpServer::newConnection(int sockFd, sockaddr_in addr)
    {
        //新连接来了，就把channel保存起来
        //channel以智能指针的形式在vec中保存，从vec中移除会使其引用计数为0,析构
        m_ptrEventLoop->assertInLoopThread();
        auto loop=m_ptrEventLoopThreadPool->getNextLoop();
        TcpConnectionPtr coon(new Channel(loop,sockFd,addr));
        {
            struct sockaddr_in6 localaddr;
            memset(&localaddr, 0, sizeof localaddr);
            socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
            if (::getsockname(sockFd, (sockaddr*)&localaddr, &addrlen) < 0)
            {
                printf("get addr error\n");
            }
        }
        m_vecConnection.push_back(coon);
        coon->setConnectionCallback(connectionCallback_);
        //设置这个新连接的读回调，这里就是业务处理，对于库的使用者，需要在这个函数中处理业务
        coon->setReadCallback(readCallBack_);
        coon->tie(coon);
        //设置关闭时的回调，就是从自己的vec中移除
        coon->setCloseCallback(std::bind(&TcpServer::removeConnection,this,std::placeholders::_1));
        //允许读事件
        loop->runInLoop(std::bind(&Channel::connectionEstablished,coon.get()));
    }
}