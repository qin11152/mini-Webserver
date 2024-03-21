/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-05-18 16:39:04
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-12 11:44:07
 * @FilePath: /MyServer/net/Acceptor.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "net/Channel.h"
#include "net/Callbacks.h"
#include <netinet/in.h>
#include <sys/socket.h>

namespace net
{
    class EventLoop;

    class Acceptor
    {
    public:
        Acceptor(EventLoop* loop,int port,const struct sockaddr_in& addr);
        ~Acceptor();

        //设置新连接时的回调处理函数
        void setNewConnectionCB(const NewConnectionCB& cb){m_newConnectionCB=cb;}

        /// @brief acceptr开始listen，接受新连接
        void listen();
        bool isListening() const {return m_bListing;}

    private:
        void handleRead(const TcpConnectionPtr& coon);

        //事件循环对象
        EventLoop* m_ptrLoop{nullptr};
        //acceptor对应的文件描述符
        int m_iSocketId{-1};
        //acceptor对应的channel
        std::shared_ptr<Channel> m_acceptChannel;
        //新连接事件回调
        NewConnectionCB m_newConnectionCB;
        bool m_bListing{false};
        int m_iIdleFd;
    };
}