#pragma once

#include <string>
#include <vector>
#include "user.hpp"
using namespace std;


class UserModel
{
public:
    // UserModel()
    // {

    // }
    // 用户注册
    bool insert(User &user);
    // 根据用户id查询用户信息
    User query(int id);
    // 更新用户状态信息
    bool updateState(User user);
    // 重置用户状态信息
    void resetState();
};
