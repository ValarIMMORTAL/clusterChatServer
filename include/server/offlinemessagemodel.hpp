#pragma once

#include <string>
#include <vector>
#include "json.hpp"
#include "public.hpp"
using json = nlohmann::json;

class OfflineMessageModel
{
public:
    // 存储用户的离线消息
    void insert(int userid, std::string msg);
    // 删除用户的离线消息
    void remove(int userid);
    // 查询用户的离线消息
    std::vector<std::string> query(int userid);
    // 移除用户的所有离线消息
    void removeAll(int userid);
};
