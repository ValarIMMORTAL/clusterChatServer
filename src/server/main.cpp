#include "chatServer.hpp"
#include "chatService.hpp"
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

int main()
{
    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress ip("127.0.0.1", 8080);
    ChatServer server(&loop, ip, "ChatServer");
    server.start(); 
    loop.loop();
    return 0;
}