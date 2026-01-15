#include "Database.h"

#include <muduo/base/Logging.h>

static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string database = "chat";

MySQL::MySQL()
{   
    // 初始化连接
    _conn = mysql_init(nullptr);
}
MySQL::~MySQL()
{
    // 关闭连接
    if (_conn != nullptr)
    {
        mysql_close(_conn);
    }
}

bool MySQL::connect()  // 连接数据库
{
    // 连接数据库
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                                    password.c_str(), database.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        // C和C++默认的编码是ASCII码，中文是乱码
        mysql_query(_conn, "SET NAMES utf8mb4");
        mysql_set_character_set(_conn, "utf8mb4");
        LOG_INFO << "MySQL Connect Success!";
        return true;
    }
    LOG_INFO << "MySQL Connect Failed!";
    return false;
}

bool MySQL::update(string sql)  // 更新操作
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << " : " << __LINE__ << " : " << sql << " : " << mysql_error(_conn) << "更新失败！";
        return false;
    }
    return true;
}

MYSQL_RES* MySQL::query(string sql)  // 查询操作
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << " : " << __LINE__ << " : " << sql << " : " << mysql_error(_conn) << "查询失败！";
        return nullptr;
    }
    return mysql_store_result(_conn);
}

MYSQL* MySQL::getConnection()  // 获取MySQL连接指针
{
    return _conn;
}