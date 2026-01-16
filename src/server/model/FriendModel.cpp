#include "FriendModel.h"
#include "Database.h"

void FriendModel::insert(int userId, int friendId)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into Friend values(%d, '%d')", userId, friendId);
    MySQL mysql;
    if (mysql.connect()) 
    {
        mysql.update(sql);
    }
}

vector<User> FriendModel::query(int userId)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.name, a.state from User a inner join Friend b on b.friendId = a.id where b.userId=%d", userId);
    vector<User> vec;
    MySQL mysql;
    if (mysql.connect()) 
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {   
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}