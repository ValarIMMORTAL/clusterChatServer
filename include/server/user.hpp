#pragma once
#include <string> 
using namespace std;

class User
{
private:
    int id;
    string name;
    string password;
    string state;

public:
    // 构造函数
    User(int id = -1, string name = "",string password = "", string state = "offline")
    {
        this->id = id;
        this->name = name;
        this->password = password;
        this->state = state;
    } 

    // Setter 方法
    void setId(int newId) {
        this->id = newId;
    }

    void setName(const string& newName) {
        this->name = newName;
    }

    void setPassword(const string& newPassword) {
        this->password = newPassword;
    }

    void setState(const string& newState) {
        this->state = newState;
    }

    // Getter 方法
    int getId() const {
        return this->id;
    }

    string getName() const {
        return this->name;
    }

    string getPassword() const {
        return this->password;
    }

    string getState() const {
        return this->state;
    }
};