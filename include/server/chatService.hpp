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

using MsgHandler = std::function<void(const TcpConnectionPtr& conn, json& js, Timestamp time)>;

/**
 * @brief 聊天服务类，负责处理聊天相关的业务逻辑
 * 
 * 该类提供了消息处理的注册和分发功能，能够根据不同的消息类型调用相应的处理函数。
 */
class ChatService
{
private:
    unordered_map<int, MsgHandler> _msgHandlerMap;
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    std::mutex _connMutex;
    // 数据操作类对象
    UserModel _userModel;
    OfflineMessageModel _offlineMsgModel;
    ChatService();
    ~ChatService();
public:
    static ChatService* instance();
    MsgHandler getHandler(int msgid);
    void login(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void clientCloseException(const TcpConnectionPtr& conn);
};
