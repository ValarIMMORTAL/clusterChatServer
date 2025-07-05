#include "offlineMessageModel.hpp"
#include <muduo/base/Logging.h>
#include "db.h"

// 存储用户的离线消息
void OfflineMessageModel::insert(int userid, std::string msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into offlineMessage values(%d, '%s')", userid, msg.c_str());
    
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            LOG_INFO << "insert offline message success";
        }
    }
}

// 删除用户的离线消息
void OfflineMessageModel::remove(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from offlineMessage where userid = %d", userid);
    
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            LOG_INFO << "delete offline message success";
        }
    }
}

// 查询用户的离线消息
std::vector<std::string> OfflineMessageModel::query(int userid)
{
    // 从用户的离线消息列表中查询指定用户的所有消息
    std::vector<std::string> msgs;
    char sql[1024] = {0};
    sprintf(sql, "select message from offlineMessage where userid = %d", userid);
    
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                msgs.push_back(row[0]);
            }
            mysql_free_result(res);
        }
    }
    return msgs;
}


