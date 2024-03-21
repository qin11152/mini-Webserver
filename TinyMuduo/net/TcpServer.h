/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-05-18 16:39:04
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-09 17:09:03
 * @FilePath: /MyServer/net/TcpServer.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#pragma once

#include "net/Channel.h"
#include "net/EventLoopThreadPool.h"

#include <atomic>
#include <string>
#include <vector>
#include <netinet/in.h>
#include <sys/socket.h>


namespace net
{
    class Acceptor;
    class EventLoop;

    //using ConnectionCallback=std::function<void (const TcpConnectionPtr&)>;

    class TcpServer
    {
    public:
        TcpServer(EventLoop* loop,const struct sockaddr_in& addr,int port);
        ~TcpServer();

        void start();

        /// @brief 设定accpetor对于新连接处理的回调函数
        /// @param cb 
        void setConnectionCallBack(const ConnectionCallback& cb){connectionCallback_=cb;}
        
        /// @brief 对于每一个连接，可以理解为一个客户连到服务器，设置它的读回调函数
        /// @param cb 
        void setReadCallBack(const ReadEventCallback& cb){readCallBack_=cb;}

        /// @brief 调用下方的runremoveinloop
        /// @param coon 要移除的channel的智能指针
        void removeConnection(const std::shared_ptr<Channel>& coon);
        /// @brief 从保存的连接vector中移除此channel，并调用channel的destroy，从epoll中移除
        /// @param coon channel对应的指针
        void removeConnectionInLoop(const std::shared_ptr<Channel>& coon);
    private:
        void newConnection(int sockFd,sockaddr_in addr);

    private:
        EventLoop* m_ptrEventLoop{nullptr};
        std::shared_ptr<EventLoopThreadPool> m_ptrEventLoopThreadPool;
        const std::string m_strIpPort{""};
        std::unique_ptr<Acceptor> m_ptrAcceptor;
        std::vector<std::shared_ptr<Channel>> m_vecConnection;
        ConnectionCallback connectionCallback_;
        ReadEventCallback readCallBack_;
    };
}