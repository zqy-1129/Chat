#pragma once

#include <string>
using namespace std;

// User表的数据映射类
class User 
{
public:
    User(int id = -1, const string& name = "", const string& password = "", const string& state = "offline")
        : id(id), name(name), password(password), state(state) {}

    void setId(int newId) { id = newId; }
    void setName(const string& newName) { name = newName; }
    void setPassword(const string& newPassword) { password = newPassword; }
    void setState(const string& newState) { state = newState; }
    
    int getId() const { return id; }
    string getName() const { return name; }
    string getPassword() const { return password; }
    string getState() const { return state; }

private:
    int id;
    string name;
    string password;
    string state;
};