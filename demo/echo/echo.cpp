/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-09 16:26:07
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-07-06 10:23:43
 * @FilePath: /MyServer/demo/echo/echo.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "echo.h"
#include <arpa/inet.h>

EcohServer::EcohServer(EventLoop* loop,sockaddr_in listenAddr,int port)
:m_tcpServer(loop,listenAddr,port)
{
    m_tcpServer.setConnectionCallBack(std::bind(&EcohServer::onConnection,this,std::placeholders::_1));
    m_tcpServer.setReadCallBack(std::bind(&EcohServer::onMessage,this,std::placeholders::_1));
}

void EcohServer::start()
{
    m_tcpServer.start();
}

void EcohServer::onConnection(const std::shared_ptr<Channel> &coon)
{
    auto addr=coon->getPeerAddr();
    char buf[100]{0};
    ::inet_ntop(AF_INET, &addr.sin_addr, buf, static_cast<socklen_t>(100));
    //printf("client ip:%s,port is:%d\n",buf,addr.sin_port);
}

void EcohServer::onMessage(const std::shared_ptr<Channel> &coon)
{
    int savedError=0;
    ssize_t n=coon->readIntoBuffer(savedError);
    if(n>0)
    {
        auto str=coon->getRecvBuffer().retrieveAllAsString();
        printf("echo server recv:%s\n",str.c_str());
        //coon->shutDownInLoop();
        coon->send(std::move(str));
    }
    else if(0==n)
    {
        coon->handleClose();
    }
    else
    {
        errno=savedError;
        printf("handle read error\n");
    }
    // auto str=coon->getRecvBuffer().retrieveAllAsString();

}
