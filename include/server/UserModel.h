#pragma once

#include "User.hpp"

#include <string>

// User表的数据操作类
class UserModel
{
public:
    // User表的插入方法
    bool insert(User& user);

    // 查询用户信息
    User query(int id);

    // 更新用户状态信息
    bool updateState(User user);

private:
};