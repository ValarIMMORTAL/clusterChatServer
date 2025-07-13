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
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGOUT_MSG, std::bind(&ChatService::loginOut, this, _1, _2, _3)});
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
        res["errno"] = 1;
        conn->send(res.dump());
    }
    else if (user.getPassword() == pwd)
    {
        if (user.getState() == "online")
        {
            json res;
            res["msgid"] = LOGIN_MSG_ACK;
            res["errno"] = 2;
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
            res["errno"] = 0;
            res["id"] = user.getId();
            res["name"] = user.getName();
            
            // 查询用户是否有离线消息
            vector<string> msg = _offlineMsgModel.query(user.getId());
            if(!msg.empty())
            {
                res["offlinemsg"] = msg;
                _offlineMsgModel.remove(user.getId());
            }
            // 查询用户的好友信息
            vector<User> friends = _friendModel.query(user.getId());
            if(!friends.empty())
            {
                vector<string> friendsData;
                for(User& user : friends)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    friendsData.push_back(js.dump());
                }
                res["friends"] = friendsData;
            }

            // 查询用户的群组信息
            vector<Group> groupUser = _groupModel.queryGroups(user.getId());
            if(!groupUser.empty())
            {
                vector<string> groupV;
                for(Group& group : groupUser)
                {
                    json groupJson;
                    groupJson["id"] = group.getId();
                    groupJson["groupname"] = group.getName();
                    groupJson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for(GroupUser& gUser: group.getUsers())
                    {
                        json gUserJs;
                        gUserJs["id"] = gUser.getId();
                        gUserJs["name"] = gUser.getName();
                        gUserJs["state"] = gUser.getState();
                        gUserJs["role"] = gUser.getRole();
                        userV.push_back(gUserJs.dump());
                    }
                    groupJson["users"] = userV;
                    groupV.push_back(groupJson.dump());
                }
                res["groups"] = groupV;
            }
            conn->send(res.dump());
        }
    }
    else
    {
        json res;
        res["msgid"] = LOGIN_MSG_ACK;
        res["errno"] = 2;
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
        res["errno"] = 1;
        res["id"] = user.getId();
        conn->send(res.dump());
    }
    else
    {
        LOG_INFO << "User registration failed";
        json res;
        res["msgid"] = REG_MSG_ACK;
        res["errno"] = 0;
        conn->send(res.dump());
    }
}

// 一对一聊天
void ChatService::oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int toId = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toId);
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
            res["errno"] = 1;
            res["errmsg"] = "对方不在线，消息已保存";
            _offlineMsgModel.insert(toId, js.dump());
            conn->send(res.dump());
        }
    }
}

// 添加好友
void ChatService::addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int userId = js["userid"].get<int>();
    int friendId = js["friendid"].get<int>();

    // 存储好友信息
    _friendModel.insert(userId, friendId);
}

// 创建群组
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userId = js["userid"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    Group group(-1, name, desc);
    if(_groupModel.createGroup(group))
    {
        // 加入群组
        _groupModel.addGroup(userId, group.getId(), "creator");
    }
}
// 加入群组
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}

// 群组聊天
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);
    for(int id : useridVec)
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(id);
        if(it != _userConnMap.end())
        {
            // 转发消息
            it->second->send(js.dump());
        }
        else
        {
            // 存储离线消息
            _offlineMsgModel.insert(id, js.dump());
        }
    }
}

// 群组聊天
void ChatService::loginOut(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        auto it = _userConnMap.find(id);
        if(it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }
    
    User user(id);
    _userModel.updateState(user);
    
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