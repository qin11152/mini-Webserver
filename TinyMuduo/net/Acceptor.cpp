/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-05-18 16:39:04
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-15 14:56:02
 * @FilePath: /MyServer/net/Acceptor.cpp
 * @Description: acceptor是用来处理新连接的，通过读事件的回调处理新连接
 */
#include "Acceptor.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace net
{
    Acceptor::Acceptor(EventLoop *loop, int port, const struct sockaddr_in &addr)
    :m_iIdleFd(::open("/dev/null",O_RDONLY | O_CLOEXEC)),
    m_acceptChannel(new Channel(loop,m_iSocketId,sockaddr_in()))
    {
        //acceptor的socket
        m_iSocketId=::socket(addr.sin_family ,SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
        if(m_iSocketId<1)
        {
            printf("create socket failed\n");
        }
        //printf("socket:%d\n",m_iSocketId);
        //设置对应的文件描述符
        m_acceptChannel->setFd(m_iSocketId);
        int optval = 1;
        //调用bind前设置，即端口释放后立即可以被使用
        setsockopt(m_iSocketId,SOL_SOCKET,SO_REUSEADDR,&optval,static_cast<socklen_t>(sizeof optval));
        setsockopt(m_iSocketId,SOL_SOCKET,SO_REUSEPORT,&optval,static_cast<socklen_t>(sizeof optval));

        int ret=bind(m_iSocketId,(struct sockaddr *)&addr,sizeof(struct sockaddr_in));
        if(ret<0)
        {
            printf("bind failed,val:%d\n",ret);
        }
        //acceptor可读时的回调，也就是有新连接时的处理
        m_acceptChannel->setReadCallback(std::bind(&Acceptor::handleRead,this,std::placeholders::_1));
    }
    Acceptor::~Acceptor()
    {
        m_acceptChannel->disableAll();
        m_acceptChannel->remove();
        close(m_iIdleFd);
    }
    void Acceptor::listen()
    {
        m_bListing=true;
        int ret=::listen(m_iSocketId,10);
        if(ret<0)
        {
            printf("listen failed\n");
        }
        m_acceptChannel->enableReading();
    }
    
    void Acceptor::handleRead(const TcpConnectionPtr& coon)
    {
        struct sockaddr_in clientAddr;
        int len = sizeof(struct sockaddr_in);
        int clientFd=::accept(m_iSocketId,(struct sockaddr*)&clientAddr,(socklen_t*)&len);
        //来了新连接就掉用对应的回调函数处理
        if(clientFd>0)
        {
            m_newConnectionCB(clientFd,clientAddr);
        }
        printf("new connection\n");
    }
}