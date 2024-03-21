/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-02 14:06:12
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-09 17:07:48
 * @FilePath: /MyServer/net/Callbacks.h
 * @Description: 所有回调函数的封装
 */
#pragma once

#include <memory>
#include <functional>
#include <netinet/in.h>

using Functor=std::function<void()>;
using TimerCallback=std::function<void()>;
using NewConnectionCB=std::function<void(int sockfd,sockaddr_in addr)>;
using EventCallBack=std::function<void()>;
using ReadEventCallback=std::function<void(std::string& msg)>;
