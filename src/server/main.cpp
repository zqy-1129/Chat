#include "ChatServer.h"

#include <iostream>
using namespace std;

int main()
{   
    EventLoop loop;
    InetAddress listenAddr("127.0.0.1", 8888);
    ChatServer server(&loop, listenAddr, "ChatServer");
    server.start();
    loop.loop();
    return 0;
}