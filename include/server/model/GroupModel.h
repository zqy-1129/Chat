#pragma once

#include "Group.hpp"

#include <string>
#include <vector>
using namespace std;

class GroupModel
{
public:
    bool createGroup(Group &group);

    void addGroup(int userId, int groupId, string role);

    vector<Group> queryGroups(int userId);

    vector<int> queryGroupUsers(int userId, int groupId);
};