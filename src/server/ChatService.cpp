#include "ChatService.h"
#include "public.h" 

#include <muduo/base/Logging.h>
#include <vector>

using namespace muduo;
using namespace std;

// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 注册消息id和对应的业务处理方法
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, bind(&ChatService::oneChat, this, _1, _2, _3)});
}

void ChatService::login(const TcpConnectionPtr &conn,
    json &js,
    Timestamp time)
{
    int id = js["id"];
    string password = js["password"];

    User user = _userModel.query(id);
    if (user.getId() != -1 && user.getPassword() == password)
    {       
        // 用于已经登录，不允许重复登录
        if (user.getState() == "online")
        {
            // 该用户不存在，登录失败
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该用户以登录，登录失败！";
            conn->send(response.dump());
        }
        else
        {   
            // 登录成功
            // 更新状态信息
            user.setState("online");
            _userModel.updateState(user);
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["id"] = user.getId();
            response["errno"] = 0;
            response["name"] = user.getName();

            // 添加用户到用户连接集合中
            {   
                // 这里使用互斥锁保证线程安全，除了作用域锁自动释放
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }

            // 查询是否有离线消息
            vector<string> vec = _offlineMessageModel.query(id);
            if (!vec.empty())
            {
                response["offineMessage"] = vec;
                // 读取完之后，则删除离线消息
                _offlineMessageModel.remove(id);
            }

            conn->send(response.dump());
        }
    }
    else 
    {    
        if (user.getId() == -1) 
        {
            // 该用户不存在，登录失败
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 1;
            response["errmsg"] = "该用户不存在，登录失败！";
            conn->send(response.dump());
        }
        else 
        {
            // 密码错误，登录失败
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 1;
            response["errmsg"] = "密码错误，登录失败！";
            conn->send(response.dump());
        }
        
    }

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

void ChatService::oneChat(const TcpConnectionPtr &conn, 
    json &js, 
    Timestamp time)
{
    int toId = js["to"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toId);
        if (it != _userConnMap.end()) 
        {
            // 在线，转发消息
            // 发消息到服务器服务器在转发到对应的收件用户
            it->second->send(js.dump());
            return;
        }
    }
    // 不再线，离线存储
    _offlineMessageModel.insert(toId, js.dump());
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{   
    User user;
    {   
        // 这里使用互斥锁保证线程安全，除了作用域锁自动释放
        lock_guard<mutex> lock(_connMutex);
        
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); it++)
        {
            if (it->second == conn)
            {   
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    // 更新用户的状态信息
    if (user.getId() != -1) 
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

void ChatService::reset()
{
    // 把online状态的用户重置
    _userModel.resetState();
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