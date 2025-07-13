#pragma once
#include <string>
#include <vector>
#include "userModel.hpp"
#include "group.hpp"

using namespace std;

class GroupModel
{
public:
    // 创建群组
    bool createGroup(Group &group);
    // 加入群组
    bool addGroup(int userId, int groupId, string role);
    // 查询用户所在群组信息
    vector<Group> queryGroups(int userId);
    // 根据指定的groupid查询群组用户id列表， 方便群聊时向群组内成员发送消息
    vector<int> queryGroupUsers(int userId, int groupId);
};
