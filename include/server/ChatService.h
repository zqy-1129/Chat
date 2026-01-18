#pragma once

#include "json.hpp"
#include "Redis.h"
#include "UserModel.h"
#include "OfflineMessageModel.h"
#include "FriendModel.h"
#include "GroupModel.h"

#include <muduo/net/TcpConnection.h>
#include <mutex>
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
    
    // 添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 一对一聊天
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 创建群组
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 加入群组
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 群聊
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 退出
    void quit(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);

    // 服务器异常关闭，业务重置
    void reset();

    // 处理订阅消息的回调
    void handleRedisSubscribeMessage(int userId, string msg);

    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);

private:
    // 单例模式构造函数私有化
    ChatService();

    // 存储消息id和对应的业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;

    // 存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;

    // 互斥锁，保证线程安全
    mutex _connMutex;

    // 数据操作类对象
    UserModel _userModel;
    OfflineMessageModel _offlineMessageModel;
    FriendModel _friendModel;
    GroupModel _groupModel;
    
    // Redis对象
    Redis _redis;
};