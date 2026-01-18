#include "ChatServer.h"
#include "ChatService.h"

#include <iostream>
#include <signal.h>
using namespace std;

// 处理服务器ctrl+c结束后，重置user状态信息
void resetHandler(int)
{
    ChatService::instance()->reset();
}

int main(int argc, char **argv)
{   
    signal(SIGINT, resetHandler);

    EventLoop loop;
    char *ip = argv[1];
    uint16_t port = argc >= 3 ? atoi(argv[2]) : 8888;
    InetAddress listenAddr(ip, port);
    ChatServer server(&loop, listenAddr, "ChatServer");
    server.start();
    loop.loop();
    return 0;
}