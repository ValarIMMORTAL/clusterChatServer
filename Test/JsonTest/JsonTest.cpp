#include "json.hpp"
#include <iostream>
#include <string>
#include <chrono>
using json = nlohmann::json;
using namespace std;

string JsonSerialize()
{
    json js;
    js["name"] = "JsonSerialize";
    js["gender"] = "male";
    js["age"] = 20;
    js["id"] = {1, 2, 3, 4, 5};
    js["books"] = {
        {"c++", 100},
        {"python", 90}
    };
    js["cars"] = {
        {"bmw", 100000},
        {"benz", 90000}
    };
    string sendBuffer = js.dump();
    return sendBuffer;
}
int main() 
{
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    tm* localTime = localtime(&time);
    // 格式化时间
    char buffer[50];
    strftime(buffer, sizeof(buffer), "%Y年%m月%d日%H:%M:%S", localTime);
    cout << "格式化时间：" << buffer << endl;
    string sendBuffer = JsonSerialize();
    json js = json::parse(sendBuffer);
    cout << js["name"]<< "\n";
    cout << js["gender"]<< "\n";
    cout << js["age"]<< "\n";
    cout << js["ids"]<< "\n";
    
    cout << js << endl;
    return 0;
}