/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-05-18 16:39:04
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-15 10:30:50
 * @FilePath: /MyServer/net/Channel.h
 * @Description: 对socket的一层封装，包含了此socket当前关注的事件，读写事件处理函数等
 */
#pragma once

#include "boost/noncopyable.hpp"
#include "net/Callbacks.h"
#include "base/Buffer.h"


namespace net
{
    class EventLoop;
    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
    
    class Channel : public boost::noncopyable, public std::enable_shared_from_this<Channel>
    {
    public:

        explicit Channel(EventLoop* loop,int fd,sockaddr_in addr);
        ~Channel();

        base::Buffer& getRecvBuffer(){return m_inputBuffer;}

        void handleEvent();
        /// @brief 读事件的处理,字节大于0就调用readcallback，=0认为断开，调用handleclose
        void handleRead();
        /// @brief 处理关闭事件，状态变为disconnect，从epoll中移除关注的事件，调用closecallback
        void handleClose();
        void handleWrite();

        /// @brief 连接建立的时候服务器会调用一次，enablereading，开始业务
        void connectionEstablished();
        /// @brief 服务器主动关闭的时候会调用
        void connectionDestreyed();

        void send(const void* message,int len);
        void send(const std::string& message);

        int readIntoBuffer(int& savedError);

        EventLoop* getLoop()const{return m_eventLoop;}
        sockaddr_in getPeerAddr()const{return m_peerAddr;}

        /// @brief 读事件的回调设置
        /// @param cb 回调函数
        void setReadCallback(std::function<void(const std::shared_ptr<Channel>&)> cb){
            m_readCB=std::move(cb);
        }

        /// @brief 写事件的回调函数设置
        /// @param cb 具体的回调函数
        void setWriteCallback(std::function<void(const std::shared_ptr<Channel>&)> cb){
            m_writeCB=std::move(cb);
        }

        void setConnectionCallback(std::function<void (const std::shared_ptr<Channel>&)> cb){
            m_connectionCallback=cb;
        }

        /// @brief 关闭事件回调函数设置
        /// @param cb 具体的回调函数
        void setCloseCallback(std::function<void(const std::shared_ptr<Channel>&)> cb){
            m_closeCB=std::move(cb);
        }
        void setErrorCallback(EventCallBack cb)
        {
            m_errorCB=std::move(cb);
        }

        void tie(const std::shared_ptr<void>&);

        /// @brief 获取此chanel对应的文件描述符
        /// @return fd
        int fd() const{return m_fd;}

        /// @brief 设置此channel的文件描述符
        /// @param fd 描述符
        void setFd(int fd){m_fd=fd;}

        /// @brief 获取当前channel关注的事件
        /// @return 
        int events()const {return m_events;}

        void setCurrentEvents(int curEvent){m_currentEvent=curEvent;}

        /// @brief 判断channel的事件是否为空，什么都不关注
        /// @return 
        bool isNoneEvent(){return kNoneEvent==m_events;}

        void enableReading(){m_events|=kReadEvent;update();}
        void disableReading(){m_events&=~kReadEvent;update();}
        void enableWriting(){m_events|=kWriteEvent;update();}
        void disableWriting(){m_events&=~kWriteEvent;update();}
        void disableAll(){m_events=kNoneEvent;update();}
        /// @brief 查看epoll中注册的事件是否有写
        /// @return 
        bool isWriting()const{return m_events&kWriteEvent;}
        bool isReading()const {return m_events&kReadEvent;}

        int index()const{return m_index;}
        void setIndex(int index){m_index=index;}

        void remove();

        void shutDownInLoop();

    private:
        void sendInLoop(const std::string& message);
        void sendInLoop(const void* message,size_t len);

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        /// @brief 调用eventloop的updatechannel，最终调用到poller中的updatechannel，用来更新此channel在epoll中关注的事件
        /// 或从epoll中添加删除此channel
        void update();

        /// @brief 事件激活后使用对应的回调函数处理
        void handleEventWithGuard();
    private:
        EventLoop* m_eventLoop;
        int m_fd;     //
        int m_events;       //
        int m_currentEvent; //
        int m_index;        //
        base::Buffer m_inputBuffer;
        base::Buffer m_outputBuffer;

        //标识这个连接当前处于什么状态
        StateE m_state;

        const sockaddr_in m_peerAddr;

        std::weak_ptr<void> m_tie;
        bool m_bTied{false};
        bool m_bEventHandling{false};
        bool m_bAddedToLoop{false};
        std::function<void(const std::shared_ptr<Channel>&)> m_readCB;
        std::function<void(const std::shared_ptr<Channel>&)> m_writeCB;
        std::function<void(const std::shared_ptr<Channel>&)> m_closeCB;
        std::function<void (const std::shared_ptr<Channel>&)> m_connectionCallback;
        EventCallBack m_errorCB;
    };  

    using TcpConnectionPtr=std::shared_ptr<Channel>;
    using ThreadInitCallback= std::function<void(EventLoop*)>;
    using ConnectionCallback= std::function<void(const TcpConnectionPtr&)>;
    using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
    using CloseCallback=std::function<void(const TcpConnectionPtr&)>;
    using ReadEventCallback=std::function<void(const TcpConnectionPtr&)>;
} // namespace net
