#include "chatService.hpp"
#include "chatServer.hpp"
#include "public.hpp"

using json = nlohmann::json;
using namespace std;
using namespace placeholders;

ChatService::ChatService(/* args */)
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({CLIENT_EXIT, std::bind(&ChatService::clientCloseException, this, _1)});
}

/**
 * @brief ChatService 类的析构函数
 * 
 * 清理聊天服务实例，释放占用的资源。
 */
ChatService::~ChatService()
{
}

// 服务器异常，业务重置
void ChatService::reset()
{
    _userModel.resetState();
}

// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // No handler found for the given message ID
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            LOG_ERROR << "This msgid:" << msgid << " has no handler";
        };
    }
    return _msgHandlerMap[msgid]; // Return the corresponding message handler
}

// 处理登录业务
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "User login: " << js.dump().c_str();
    string name = js["name"];
    string pwd = js["password"];
    User user = _userModel.query(name);
    if (user.getId() == -1)
    {
        LOG_INFO << "User login failed";
        json res;
        res["msgid"] = LOGIN_MSG_ACK;
        res["error"] = 1;
        conn->send(res.dump());
    }
    else if (user.getPassword() == pwd)
    {
        if (user.getState() == "online")
        {
            json res;
            res["msgid"] = LOGIN_MSG_ACK;
            res["error"] = 2;
            res["errmsg"] = "已登录，不可重复登录";
            conn->send(res.dump());
        }
        else
        {
            // 登录成功
            {
                std::lock_guard<std::mutex> lock(_connMutex);
                _userConnMap.insert({user.getId(), conn});
            }
            // 更新用户状态为在线
            user.setState("online");
            _userModel.updateState(user);
            json res;
            res["msgid"] = LOGIN_MSG_ACK;
            res["error"] = 0;
            res["id"] = user.getId();
            
            // 查询用户是否有离线消息
            vector<string> msg = _offlineMsgModel.query(user.getId());
            if(!msg.empty())
            {
                res["offlinemsg"] = msg;
                _offlineMsgModel.remove(user.getId());
            }

            conn->send(res.dump());
        }
    }
    else
    {
        json res;
        res["msgid"] = LOGIN_MSG_ACK;
        res["error"] = 2;
        res["errmsg"] = "密码错误";
        conn->send(res.dump());
    }
}

// 处理注册业务
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // Handle registration logic here
    LOG_INFO << "User registration: " << js.dump().c_str();
    string name = js["name"];
    string pwd = js["password"];
    User user;
    user.setName(name);
    user.setPassword(pwd);
    bool state = _userModel.insert(user);
    if (state)
    {
        LOG_INFO << "User registration success";
        json res;
        res["msgid"] = REG_MSG_ACK;
        res["error"] = 0;
        res["id"] = user.getId();
        conn->send(res.dump());
    }
    else
    {
        LOG_INFO << "User registration failed";
        json res;
        res["msgid"] = REG_MSG_ACK;
        res["error"] = 1;
        conn->send(res.dump());
    }
}

// 一对一聊天
void ChatService::oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int destId = js["destId"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(destId);
        if(it != _userConnMap.end())
        {
            // 找到用户
            it->second->send(js.dump());
        }
        else
        {
            // 没找到用户
            json res;
            res["msgid"] = ONE_CHAT_MSG_ACK;
            res["error"] = 1;
            res["errmsg"] = "对方不在线，消息已保存";
            _offlineMsgModel.insert(destId, js.dump());
            conn->send(res.dump());
        }
    }
}

// 处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr& conn)
{
    User user;
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        for(auto& userConn : _userConnMap)
        {
            if(userConn.second != conn)
                continue;
            user.setId(userConn.first);
            _userConnMap.erase(userConn.first);
            break;
        }
    }
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }

}