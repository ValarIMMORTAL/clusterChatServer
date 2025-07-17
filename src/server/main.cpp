#include "chatServer.hpp"
#include "chatService.hpp"
#include <iostream>
#include "signal.h"
using namespace std;

// 服务器异常退出时，重置所有用户的状态信息
void resetHandler(int sig)
{
    // 打印信号处理函数被调用的日志信息
    LOG_INFO << "resetHandler is called, sig = " << sig;
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./clusterChatServer 127.0.0.1 8080" << endl;
        exit(-1);
    }

    // 获取命令行参数的IP和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);
    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress ipAdd(ip, port);
    ChatServer server(&loop, ipAdd, "ChatServer");
    server.start(); 
    loop.loop();
    return 0;
}