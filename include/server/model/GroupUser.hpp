#pragma once

#include "User.hpp"

class GroupUser : public User
{
public:
    void setRole(string newRole) { role = newRole; }
    string getRole() { return role; } 

private:
    string role;
    
};