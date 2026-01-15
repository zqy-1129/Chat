#include "ChatServer.h"
#include "ChatService.h"
#include "json.hpp"

#include <string>
#include <functional>
#include <muduo/base/Logging.h>

using namespace std;
using json = nlohmann::json;
using namespace placeholders;
using namespace muduo;

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
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr &conn,
    Buffer *buf,
    Timestamp time)
{   
    // string msg = buf->retrieveAllAsString();
    // // 解析json数据
    // json js = json::parse(msg);
    // // 获取消息id
    // int msgid = js["msgid"].get<int>();
    // // 这里就实现了网络和业务的完全解耦
    // // 获取对应的消息处理器
    // auto handler = ChatService::instance()->getHandler(msgid);
    // // 调用消息处理器，进行业务处理
    // handler(conn, js, time);

    // ========== 完美健壮版 - 你的业务代码直接替换 ==========
    string msg = buf->retrieveAllAsString();
    json js;
    bool parse_ok = true;

    // 1. 第一步：捕获JSON解析异常，防止解析失败导致崩溃
    try
    {
        js = json::parse(msg);
    }
    catch(const nlohmann::json::parse_error& e)
    {
        parse_ok = false;
        LOG_ERROR << "客户端数据JSON解析失败：" << e.what() << "，原始数据：" << msg;
    }
    catch(...)
    {
        parse_ok = false;
        LOG_ERROR << "客户端数据未知解析异常，原始数据：" << msg;
    }

    // 解析失败，直接关闭连接/返回，不继续执行
    if (!parse_ok)
    {
        conn->shutdown(); // 可选：关闭无效连接，也可以conn->send错误提示
        return;
    }

    // 2. 第二步：安全判断JSON中是否存在 msgid 字段 + 是数字类型
    int msgid = -1;
    if (js.contains("msgid") && js["msgid"].is_number_integer())
    {
        msgid = js["msgid"].get<int>();
    }
    else
    {
        // 日志打印详细错误，定位问题（重中之重，调试必备）
        LOG_ERROR << "客户端JSON数据异常：缺失msgid字段 或 msgid不是整型，原始数据：" << msg;
        conn->shutdown(); // 关闭该无效连接
        return;
    }

    // 3. 第三步：安全获取消息处理器，判断回调是否有效
    auto handler = ChatService::instance()->getHandler(msgid);
    if (handler)
    {
        // 一切正常，调用处理器执行业务逻辑（你的原有逻辑不变）
        handler(conn, js, time);
    }
    else
    {
        // 异常：msgid合法，但无对应处理器（比如msgid=999，未注册）
        LOG_ERROR << "无效的msgid=" << msgid << "，无对应业务处理器，原始数据：" << msg;
        conn->shutdown();
    }
}   