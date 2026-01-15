#include "ChatService.h"
#include "public.h" 

#include <muduo/base/Logging.h>
using namespace muduo;

// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 注册消息id和对应的业务处理方法
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
}

void ChatService::login(const TcpConnectionPtr &conn,
    json &js,
    Timestamp time)
{
    LOG_INFO << "do login service!!!";
}

void ChatService::reg(const TcpConnectionPtr &conn,
    json &js,
    Timestamp time)
{
    string name = js["name"];
    string password = js["password"];

    User user;
    user.setName(name);
    user.setPassword(password);
    user.setState("online");

    if (_userModel.insert(user))
    {   
        // 注册成功，响应注册结果
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["id"] = user.getId();
        response["errno"] = 0;
        conn->send(response.dump());
    }
    else
    {
        // 注册失败，响应注册结果
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = -1;
        response["errmsg"] = "注册失败，请重试！";
        conn->send(response.dump());
    }
}

MsgHandler ChatService::getHandler(int msgid)
{   
    // 记录错误日志，msgid没有对应的处理器
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {   
        // 返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr &,
            json &,
            Timestamp)
        {
            LOG_ERROR << "msgid:" << msgid << " has no handler!";
        };
    }
    return it->second;
}