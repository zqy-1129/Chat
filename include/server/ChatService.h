#pragma once

#include "json.hpp"
#include "UserModel.h"

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

// 处理消息的事件回调方法类型
using MsgHandler = function<void(const TcpConnectionPtr &conn,
    json &js,
    Timestamp time)>;

// 完全解耦网络模块的代码和业务模块的代码
// 使用单例模式
class ChatService
{
public:
    // 获取单例对象的接口函数
    static ChatService *instance();

    // 登录
    void login(const TcpConnectionPtr &conn,
        json &js,
        Timestamp time);

    // 注册
    void reg(const TcpConnectionPtr &conn,
        json &js,
        Timestamp time);

    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);

private:
    // 单例模式构造函数私有化
    ChatService();

    // 存储消息id和对应的业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;

    // 用户数据操作类对象，负责User表的操作
    UserModel _userModel;
};