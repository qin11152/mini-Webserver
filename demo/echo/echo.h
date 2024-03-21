/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-09 16:26:01
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-14 14:18:01
 * @FilePath: /MyServer/demo/echo/echo.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include "net/TcpServer.h"

using namespace net;

class EcohServer
{
public:
    EcohServer(EventLoop* loop,sockaddr_in listenAddr,int port);

    void start();

private:

    void onConnection(const std::shared_ptr<Channel>& coon);
    void onMessage(const std::shared_ptr<Channel>& coon);

    TcpServer m_tcpServer;
};