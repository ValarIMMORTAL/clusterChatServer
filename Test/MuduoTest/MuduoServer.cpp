#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"
#include "muduo/net/InetAddress.h"
#include "muduo/base/Logging.h"
#include <iostream>
#include <string>
using namespace std;
using namespace muduo;
using namespace muduo::net;

class MuduoServer
{
public:
    MuduoServer(EventLoop *loop,
                const InetAddress &listenAddr,
                const string &nameArg)
        : server_(loop, listenAddr, nameArg)
        , loop_(loop)
    {
        server_.setConnectionCallback(
            std::bind(&MuduoServer::onConnection, this, placeholders::_1));
        server_.setMessageCallback(
            std::bind(&MuduoServer::onMessage, this, placeholders::_1, placeholders::_2, placeholders::_3));
        server_.setThreadNum(4);
    }

    void start()
    {
        server_.start();
        LOG_INFO << "Muduo Server started at " << server_.ipPort();
    }

private:
    TcpServer server_;
    EventLoop *loop_;

    // 处理连接事件
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            LOG_INFO << "New connection from " << conn->peerAddress().toIpPort()
                     << " to " << conn->localAddress().toIpPort();
            conn->send("Welcome to Muduo Server!\n");
        }
        else
        {
            LOG_INFO << "Connection closed by " << conn->peerAddress().toIpPort();
            conn->shutdown(); // 关闭连接
        }
    }

    // 处理接收到的消息
    void onMessage(const TcpConnectionPtr &conn,
                   Buffer *buf,
                   Timestamp time)
    {
        string msg = buf->retrieveAllAsString();
        LOG_INFO << "Received message: " << msg
                 << " at " << time.toString();
        conn->send("Message received: " + msg + "\n");
    }
};

int main()
{
    EventLoop loop;
    InetAddress listenAddr("127.0.0.1", 8080);
    MuduoServer server(&loop, listenAddr, "MuduoServer");

    server.start();
    loop.loop();

    return 0;
}