#pragma once

#include <mysql/mysql.h>
#include <string>
using namespace std;

class MySQL
{
public:
    MySQL();

    ~MySQL();

    bool connect();  // 连接数据库

    bool update(string sql);  // 更新操作

    MYSQL_RES* query(string sql);  // 查询操作

    MYSQL* getConnection();  // 获取MySQL连接指针

private:
    MYSQL *_conn;  // MySQL连接指针
};