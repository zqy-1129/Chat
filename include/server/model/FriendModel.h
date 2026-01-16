#pragma once

#include "User.hpp"

#include <vector>

using namespace std;

class FriendModel
{
public:
    // 添加好友
    void insert(int userId, int friendId);

    // 返回用户好友列表
    vector<User> query(int userId);

private:

};