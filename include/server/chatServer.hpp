#pragma once
#include "json.hpp"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
using namespace muduo;
using namespace muduo::net;

// 聊天服务器类，封装了基于 muduo 库的 TCP 服务器功能
class ChatServer
{
private:
    // muduo 库的 TcpServer 对象，用于处理 TCP 连接
    TcpServer _server;
    // 指向 EventLoop 对象的指针，用于事件循环
    EventLoop *_loop;

    // 处理新的 TCP 连接事件
    // @param conn 新连接的 TcpConnection 智能指针
    void onConnection(const TcpConnectionPtr &conn);

    // 处理接收到的消息事件
    // @param conn 发送消息的 TcpConnection 智能指针
    // @param buf 存储接收到消息的缓冲区指针
    // @param time 消息到达的时间戳
    void onMessage(const TcpConnectionPtr &conn,
                   Buffer *buf,
                   Timestamp time);
public:
    // 构造函数，初始化聊天服务器
    // @param loop 事件循环对象指针
    // @param listenAddr 服务器监听地址
    // @param nameArg 服务器名称
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const string &nameArg);
    // 析构函数
    ~ChatServer();
    
    // 启动聊天服务器
    void start();
};
