#pragma once
#include "json.hpp"
#include <muduo/net/TcpConnection.h>
#include <muduo/base/Logging.h>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "userModel.hpp"
#include "offlineMessageModel.hpp"

using json = nlohmann::json;
using namespace std;
using namespace muduo;
using namespace muduo::net;

using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;

/**
 * @brief 聊天服务类，负责处理聊天相关的业务逻辑
 *
 * 该类是一个单例类，提供了消息处理的注册和分发功能，能够根据不同的消息类型调用相应的处理函数。
 * 同时管理用户的连接信息，处理用户的登录、注册、聊天等业务，以及处理客户端异常退出的情况。
 */
class ChatService
{
private:
    // 存储消息Id和对应的业务
    unordered_map<int, MsgHandler> _msgHandlerMap;
    // 存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    // 读写连接的互斥锁
    std::mutex _connMutex;
    // 数据操作类对象
    UserModel _userModel;
    // 离线消息
    OfflineMessageModel _offlineMsgModel;
    ChatService();
    ~ChatService();

public:
    // 获取单例对象的接口函数
    static ChatService *instance();
    // 服务器异常，业务重置
    void reset();
    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    // 处理登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 一对一聊天
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
};
