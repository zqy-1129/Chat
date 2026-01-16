#pragma once

#include "GroupUser.hpp"

#include <string>
#include <vector>

class Group
{
public:
    Group(int id = -1, string name = "", string desc = "")
        : id(id), name(name), desc(desc) {}
    
    void setId(int newId) { id = newId; }
    void setName(const string& newName) { name = newName; }
    void setDesc(const string& newDesc) { desc = newDesc; }
    
    int getId() const { return id; }
    string getName() const { return name; }
    string getDesc() const { return desc; }
    vector<GroupUser> &getUsers() { return users; }

private:
    int id;
    string name;
    string desc;
    vector<GroupUser> users;
};