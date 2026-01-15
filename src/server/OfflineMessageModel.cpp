#include "OfflineMessageModel.h"
#include "Database.h"

OfflineMessageModel::OfflineMessageModel()
{

}
OfflineMessageModel::~OfflineMessageModel()
{

}

void OfflineMessageModel::insert(int userId, const string& message)
{   
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage(userId, message) values(%d, '%s')", userId, message.c_str());
    MySQL mysql;
    if (mysql.connect()) 
    {
        // FIX ME: NO ROBOST
        mysql.update(sql);
    }
}

void OfflineMessageModel::remove(int userId)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userId=%d", userId);
    MySQL mysql;
    if (mysql.connect()) 
    {
        // FIX ME: NO ROBOST
        mysql.update(sql);
    }
}

vector<string> OfflineMessageModel::query(int userId)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where userId=%d", userId);
    vector<string> vec;
    MySQL mysql;
    if (mysql.connect()) 
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            // 把userId用户所有离线消息返回
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}