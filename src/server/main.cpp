#include "chatServer.hpp"
using namespace std;
int main()
{
    EventLoop loop;
    InetAddress ip("127.0.0.1", 8080);
    ChatServer server(&loop, ip, "ChatServer");
    server.start(); 
    loop.loop();
    return 0;
}