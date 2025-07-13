#include "userModel.hpp"
#include <muduo/base/Logging.h>
#include "db.h"
#include "user.hpp"

bool UserModel::insert(User &user)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')",
            user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());
    MySQL mysql;
    if(!mysql.connect())
    {
        return false;
    }
    if(mysql.update(sql))
    {
        user.setId(mysql_insert_id(mysql.getConnection()));
        return true;
    }
    return false;
}

User UserModel::query(string name)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from user where name='%s'", name.c_str());
    MySQL mysql;
    if(!mysql.connect())
    {
        return false;
    }
    MYSQL_RES *res = mysql.query(sql);
    if(res != nullptr)
    {
        MYSQL_ROW row = mysql_fetch_row(res);
        if(row != nullptr)
        {
            User user;
            user.setId(atoi(row[0]));
            user.setName(row[1]);
            user.setPassword(row[2]);
            user.setState(row[3]);
            return user;
        }
    }
    return User();
}

// 更新用户状态信息
bool UserModel::updateState(User user)
{
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());
    MySQL mysql;
    if(!mysql.connect())
    {
        return false;
    }
    if(mysql.update(sql))
    {
        return true;
    }
    return false;
}

// 重置用户状态信息
void UserModel::resetState()
{
    char sql[1024] = "update user set state = 'offline' where state = 'online'";
    MySQL mysql;
    if(!mysql.connect())
    {
        return;
    }
    mysql.update(sql);
}
