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

int main()
{   
    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress listenAddr("127.0.0.1", 8888);
    ChatServer server(&loop, listenAddr, "ChatServer");
    server.start();
    loop.loop();
    return 0;
}