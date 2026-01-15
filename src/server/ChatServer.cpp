#include "ChatServer.h"
#include "ChatService.h"
#include "json.hpp"

#include <string>
#include <functional>
using namespace std;
using json = nlohmann::json;
using namespace placeholders;

ChatServer::ChatServer(EventLoop *loop,
    const InetAddress &listenAddr,
    const string &nameArg)
    : _server(loop, listenAddr, nameArg),
      _loop(loop)
{
    _server.setConnectionCallback(
        std::bind(&ChatServer::onConnection, this, _1));
    _server.setMessageCallback(
        std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    _server.setThreadNum(4);
}

void ChatServer::start()
{
    _server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &conn)
{   
    // 客户端断开连接
    if (!conn->connected())
    {
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr &conn,
    Buffer *buf,
    Timestamp time)
{   
    string msg = buf->retrieveAllAsString();
    // 解析json数据
    json js = json::parse(msg);
    // 获取消息id
    int msgid = js["msgid"].get<int>();
    // 这里就实现了网络和业务的完全解耦
    // 获取对应的消息处理器
    auto handler = ChatService::instance()->getHandler(msgid);
    // 调用消息处理器，进行业务处理
    handler(conn, js, time);
}   