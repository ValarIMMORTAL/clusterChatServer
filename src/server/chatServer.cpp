#include "chatServer.hpp"
#include "chatService.hpp"
#include "public.hpp"
#include <functional>
#include <iostream>

using namespace std;
using namespace placeholders;
using json = nlohmann::json;

/**
 * @brief ChatServer 类的构造函数
 * 
 * 初始化聊天服务器实例，设置服务器的连接回调、消息回调和线程数量。
 * 
 * @param loop 指向 EventLoop 对象的指针，用于事件循环。
 * @param listenAddr 服务器监听的网络地址，包含 IP 地址和端口号。
 * @param nameArg 服务器的名称，用于标识该服务器实例。
 */
ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg),  // 初始化 TcpServer 实例，传入事件循环、监听地址和服务器名称
      _loop(loop)  // 初始化事件循环指针
{
    // 设置连接回调函数，当有新的客户端连接或连接断开时，会调用 onConnection 方法
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
    // 设置消息回调函数，当接收到客户端发送的消息时，会调用 onMessage 方法
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
    // 设置服务器的线程数量为 4，即使用 4 个线程来处理客户端连接和消息
    _server.setThreadNum(4);
}

/**
 * @brief ChatServer 类的析构函数
 * 
 * 清理聊天服务器实例，释放占用的资源。
 */
ChatServer::~ChatServer()
{
}

/**
 * @brief 启动聊天服务器
 * 
 * 此方法调用 TcpServer 实例的 start 方法，开始监听指定地址和端口，
 * 并启动服务器的事件循环，使服务器开始接受客户端的连接请求。
 */
void ChatServer::start()
{
    // 调用 TcpServer 实例的 start 方法，启动服务器监听
    _server.start();
}

/**
 * @brief 处理客户端连接状态变化
 * 
 * 当有新的客户端连接到服务器，或者已有客户端断开连接时，此方法会被调用。
 * 根据连接状态，记录客户端的连接信息和状态变化，并在客户端断开连接时关闭连接。
 * 
 * @param conn 指向 TcpConnection 对象的智能指针，表示与客户端的连接。
 */
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    // 检查客户端连接是否处于连接状态
    if (conn->connected())
    {
        LOG_INFO << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state: online\n";
    }
    else
    {
        LOG_INFO << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state: offline\n";
        ChatService::instance()->clientCloseException(conn);
        // 关闭客户端连接
        conn->shutdown();
    }
}

/**
 * @brief 处理客户端发送的消息
 * 
 * 当服务器接收到客户端发送的消息时，此方法会被调用。
 * 它会从缓冲区中读取消息，解析为 JSON 格式，根据消息 ID 获取对应的消息处理函数，
 * 若存在处理函数则调用该函数处理消息，最后记录日志。
 * 
 * @param conn 指向 TcpConnection 对象的智能指针，表示与客户端的连接。
 * @param buffer 指向 Buffer 对象的指针，包含客户端发送的消息。
 * @param time 消息到达的时间戳。
 */
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp time)
{
    // 从缓冲区中读取所有数据，并转换为字符串
    string buf = buffer->retrieveAllAsString();
    // 将读取到的字符串解析为 JSON 对象
    json js = json::parse(buf);
    // 从 JSON 对象中获取消息 ID
    int msgid = js["msgid"].get<int>();
    // 根据消息 ID 从 ChatService 单例中获取对应的消息处理函数
    auto msgHandler = ChatService::instance()->getHandler(msgid);
    // 检查是否找到了对应的消息处理函数
    if(msgHandler == nullptr)
    {
        LOG_ERROR << "msgid:" << msgid << "can not find handler\n";
        return;
    }
    // 调用找到的消息处理函数处理消息
    msgHandler(conn, js, time);
    // 记录接收到的消息日志
    LOG_INFO << "[" << conn->name() << "] " << buf << "\n";
}
