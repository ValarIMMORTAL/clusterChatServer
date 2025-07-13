#pragma once
#include "user.hpp"

class GroupUser : public User
{
private:
    string role;
public:
    void setRole(string role)
    {
        this->role = role;
    }
    string getRole()
    {
        return this->role;
    }
};