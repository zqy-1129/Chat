#include "json.hpp"
#include "Group.hpp"
#include "User.hpp"
#include "public.h"

#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <ctime>
#include <chrono>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <limits>  // 用于cin.ignore清空缓冲区
#include <stdexcept> // 用于捕获json解析异常

using namespace std;
using json = nlohmann::json;

// 当前系统登录的用户 - 修复：命名笔误 g_currrentUser -> g_currentUser
User g_currentUser;

// 当前登录用户的好友列表信息
vector<User> g_currentUserFriendList;

// 当前登录用户的群组列表信息
vector<Group> g_currentUserGroupList;

// 控制聊天程序
bool isMainMenuRunning = false;

void help(int = 0, string = "");
void chat(int, string);
void addFriend(int, string);
void createGroup(int, string);
void addGroup(int, string);
void groupChat(int, string);
void quit(int = 0, string = "");

// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令，格式help"},
    {"chat", "一对一聊天，格式chat:friendId:message"},
    {"addFriend", "添加好友，格式addfriend:friendId"},
    {"createGroup", "创建群聊，格式createGroup:groupName:groupDesc"},
    {"addGroup", "加入群组，格式addGroup:groupId"},
    {"groupChat", "群聊，格式groupChat:groupId:message"},
    {"quit", "注销，格式quit"}
};

// 注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addFriend", addFriend},
    {"createGroup", createGroup},
    {"addGroup", addGroup},
    {"groupChat", groupChat},
    {"quit", quit}
};

// 当前用户的基本信息
void showCurrentUserData();

// 接受线程
void readTaskHandler(int clientfd);

// 获取系统时间
string getCurrentTime();

// 主聊天页面
void mainMenu(int clientfd);

// 聊天客户端程序实现，main线程用做发送线程，子线程用作接受线程
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "comman invalid! example: ./ChatClient 127.0.0.1 8888" << endl;
        exit(-1);
    }
    
    // 解析通过命令行传递参数的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 创建Client的socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        cerr << "socket create error" << endl;
        exit(-1);
    }

    // 填写client需要连接的server信息ip+port
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    // Client和server进行连接
    if (connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)) == -1)
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }

    // main线程用于接受用户输入，负责发送数据
    for (;;)
    {
        cout << "===================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "===================" << endl;
        cout << "choice:";
        int choice = 0;
        cin >> choice;

        // 修复：处理输入异常 + 重置状态 + 清空脏数据
        if (!cin) {
            cin.clear(); 
            choice = 0;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch(choice)
        {
            case 1:     // 登录
            {
                int id = 0;
                char pwd[50] = {0};
                cout << "userId:";
                cin >> id;
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 修复：清空缓冲区，解决密码输入为空
                cout << "password:";
                cin.getline(pwd, 50);

                json js;
                js["msgid"] = LOGIN_MSG;
                js["id"] = id;
                js["password"] = pwd;
                string request = js.dump();
                
                // 修复：send用size()代替strlen，安全无截断
                int len = send(clientfd, request.c_str(), request.size(), 0);
                if (len == -1)
                {
                    cerr << "send login msg error:" << request << endl;
                }
                else
                {
                    char buffer[1024] = {0};
                    len = recv(clientfd, buffer, 1024, 0);
                    if (len == -1)
                    {
                        cerr << "recv login response error" << endl;
                    }
                    else 
                    {
                        try{ // 修复：包裹所有json解析逻辑，捕获异常
                            json responsejs = json::parse(buffer);
                            if (responsejs["errno"].get<int>() != 0)
                            {
                                cerr << responsejs["errmsg"] << endl;
                            }
                            else    // 登录成功
                            {
                                // 修复：登录成功前，清空全局容器+重置用户信息，防止数据重复
                                g_currentUserFriendList.clear();
                                g_currentUserGroupList.clear();
                                g_currentUser.setId(responsejs["id"].get<int>());
                                g_currentUser.setName(responsejs["name"]);
                                // cout << responsejs.dump() << endl;

                                // 记录当前用户的好友列表 - 修复：302崩溃根因，三重校验+顺序正确+异常捕获
                                if (responsejs.contains("friends") && !responsejs["friends"].is_null() && responsejs["friends"].is_array())
                                {
                                    vector<string> vec = responsejs["friends"];
                                    for (string &str : vec)
                                    {
                                        try{
                                            json js = json::parse(str);
                                            User user;
                                            user.setId(js["id"].get<int>());
                                            user.setName(js["name"]);
                                            user.setState(js["state"]);
                                            g_currentUserFriendList.push_back(user);
                                        }catch(const exception& e){
                                            cerr << "好友信息解析失败: " << e.what() << endl;
                                        }
                                    }
                                }

                                // 记录当前用户的群组列表 - 修复：302崩溃根因，三重校验+顺序正确+异常捕获
                                if (responsejs.contains("groups") && !responsejs["groups"].is_null() && responsejs["groups"].is_array())
                                {
                                    vector<string> vec = responsejs["groups"];
                                    for (string &groupstr: vec)
                                    {
                                        try{
                                            json grpjs = json::parse(groupstr);
                                            Group group;
                                            group.setId(grpjs["id"].get<int>());
                                            group.setName(grpjs["groupName"]);
                                            group.setDesc(grpjs["groupDesc"]);

                                            if(grpjs.contains("users") && !grpjs["users"].is_null() && grpjs["users"].is_array()){
                                                vector<string> vec1 = grpjs["users"];
                                                for (string &userstr: vec1)
                                                {
                                                    json js = json::parse(userstr);
                                                    GroupUser user;
                                                    user.setId(js["id"].get<int>());
                                                    user.setName(js["name"]);
                                                    user.setState(js["state"]);
                                                    user.setRole(js["role"]);
                                                    group.getUsers().push_back(user);
                                                }
                                            }
                                            g_currentUserGroupList.push_back(group);
                                        }catch(const exception& e){
                                            cerr << "群组信息解析失败: " << e.what() << endl;
                                        }
                                    }
                                }
                            }

                            // 显示基本信息
                            showCurrentUserData();

                            // 显示当前用户的离线消息 - 修复：拼写错误 offineMessage -> offlineMessage + 三重校验 + 异常捕获
                            if (responsejs.contains("offlineMessage") && !responsejs["offlineMessage"].is_null() && responsejs["offlineMessage"].is_array())
                            {
                                vector<string> vec = responsejs["offlineMessage"];
                                for (string &msg: vec)
                                {
                                    try{
                                        json js = json::parse(msg);
                                        int msgtype = js["msgid"].get<int>();
                                        if (ONE_CHAT_MSG == msgtype)
                                        {
                                            cout << js["time"] << "[" << js["id"] <<  "]" <<  js["name"] 
                                                << "said: " << js["msg"] << endl;
                                            continue;
                                        }
                                        
                                        else if (GROUP_CHAT_MSG == msgtype)
                                        {
                                            cout << "Group MSG: " << js["time"] << "[" << js["groupId"] << ": " << js["id"] <<  "]" <<  js["name"] 
                                                << "said: " << js["msg"] << endl;
                                            continue;
                                        }
                                    }catch(const exception& e){
                                        cerr << "离线消息解析失败: " << e.what() << endl;
                                    }
                                }
                            }

                            // 登录成功，启动接受线程负责接受数据，只需启动一次
                            static int threadNumber = 0;
                            if (threadNumber == 0) {
                                std::thread readTask(readTaskHandler, clientfd);
                                readTask.detach();
                                threadNumber++;
                            }
                            
                            // 进入聊天主菜单界面
                            isMainMenuRunning = true;
                            mainMenu(clientfd);
                        }catch(const exception& e){
                            cerr << "JSON解析总异常: " << e.what() << endl;
                        }
                    }
                }

            }
            break;
            case 2:     // 注册
            {
                char name[50] = {0};
                char pwd[50] = {0};
                cout << "username:";
                cin.getline(name, 50);
                cout << "userpassword:";
                cin.getline(pwd, 50);

                json js;
                js["msgid"] = REG_MSG;
                js["name"] = name;
                js["password"] = pwd;
                string request = js.dump();

                // 修复：send用size()代替strlen
                int len = send(clientfd, request.c_str(), request.size(), 0);
                if (len == -1)
                {
                    cerr << "send reg msg error:" << request << endl;
                }
                else
                {
                    char buffer[1024] = {0};
                    len = recv(clientfd, buffer, 1024, 0);
                    if (len == -1)
                    {
                        cerr << "recv reg response error" << endl;
                    }
                    else
                    {
                        json responsejs = json::parse(buffer);
                        if (responsejs["errno"].get<int>())
                        {
                            cerr << name << " is already exist, register error!" << endl;
                        }
                        else // 注册成功
                        {
                            cout << name << " register success, userid is " << responsejs["id"]
                                << ", do not forget it!" << endl;
                        }
                    }
                }
            }
            break;
            case 3:     // 退出
                close(clientfd);
                exit(0);
            default:
                cerr << "invalid input!" << endl;
            break;
        }
    }
}

void showCurrentUserData()
{
    cout << "======================login user======================" << endl;
    cout << "current login user => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << endl;
    cout << "----------------------friend list---------------------" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user: g_currentUserFriendList)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "----------------------group list-----------------------" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (Group &group: g_currentUserGroupList)
        {
            cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
            for (GroupUser &user: group.getUsers())
            {
                cout << user.getId() << " " << user.getName() << " " << user.getState() << " " << user.getRole() << endl;
            }
        }
    }
    cout << "=====================================================" << endl;
}

void readTaskHandler(int clientfd)
{
    for (;;)
    {
        char buffer[1024] = {0}; // 修复：缓冲区移到循环内，每次清空
        int len = recv(clientfd, buffer, 1024, 0);
        // 修复：致命错误 len = -1 → len == -1
        if (len == -1 || len == 0)
        {
            cerr << "服务端断开连接，客户端退出..." << endl;
            close(clientfd);
            exit(-1);
        }

        try{
            json js = json::parse(buffer);
            if (ONE_CHAT_MSG == js["msgid"].get<int>())
            {
                cout << js["time"] << "[" << js["id"] <<  "]" <<  js["name"] 
                    << "said: " << js["msg"] << endl;
                continue;
            }

            if (GROUP_CHAT_MSG == js["msgid"].get<int>())
            {
                cout << "Group MSG: " << js["time"] << "[" << js["groupId"] << ": " << js["id"] <<  "]" <<  js["name"] 
                    << "said: " << js["msg"] << endl;
                continue;
            }
        }catch(const exception& e){
            cerr << "接收线程JSON解析失败: " << e.what() << endl;
        }
    }
}

string getCurrentTime()
{
    auto tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm *ptm = localtime(&tt);
    char time[60] = {0};
    sprintf(time, "%04d-%02d-%02d %02d:%02d:%02d",
        (int)(1900 + ptm->tm_year), (int)(1 + ptm->tm_mon), (int)ptm->tm_mday,
        (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return string(time);
}

void help(int, string)
{
    cout << "show command list" << endl;
    for (auto &p: commandMap)
    {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}

void chat(int clientfd, string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        cerr << "chat command invalid" << endl;
        return;
    }

    int friendId = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);
    if(friendId <=0 || message.empty()){ // 优化：参数校验
        cerr << "好友ID非法或消息为空" << endl;
        return;
    }

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["to"] = friendId;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), buffer.size(), 0);
    if (len == -1)
    {
        cerr << "send chat msg error ->" << buffer << endl;
    }
}

void addFriend(int clientfd, string str)
{
    int friendId = atoi(str.c_str());
    if(friendId <=0){ // 优化：参数校验
        cerr << "好友ID非法" << endl;
        return;
    }
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendId"] = friendId;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), buffer.size(), 0);
    if (len == -1)
    {
        cerr << "send addFriend msg error ->" << buffer << endl;
    }
}

void createGroup(int clientfd, string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        cerr << "createGroup command invalid" << endl;
        return;
    }
    
    string groupName = str.substr(0, idx);
    string groupDesc = str.substr(idx + 1, str.size() - idx);
    if(groupName.empty() || groupDesc.empty()){ // 优化：参数校验
        cerr << "群名或群描述不能为空" << endl;
        return;
    }

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupName"] = groupName;
    js["groupDesc"] = groupDesc;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), buffer.size(), 0);
    if (len == -1)
    {
        cerr << "send createGroup msg error ->" << buffer << endl;
    }
}

void addGroup(int clientfd, string str)
{
    int groupId = atoi(str.c_str());
    if(groupId <=0){ // 优化：参数校验
        cerr << "群组ID非法" << endl;
        return;
    }
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupId"] = groupId;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), buffer.size(), 0);
    if (len == -1)
    {
        cerr << "send addGroup msg error ->" << buffer << endl;
    }
}

void groupChat(int clientfd, string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        cerr << "groupChat command invalid" << endl;
        return;
    }
    
    int groupId = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);
    if(groupId <=0 || message.empty()){ // 优化：参数校验
        cerr << "群组ID非法或消息为空" << endl;
        return;
    }

    json js;
    // 修复：msgid错误 CREATE_GROUP_MSG → GROUP_CHAT_MSG
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupId"] = groupId;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), buffer.size(), 0);
    if (len == -1)
    {
        // 修复：日志文案错误
        cerr << "send groupChat msg error ->" << buffer << endl;
    }
}

// 修复：补充quit退出逻辑
void quit(int clientfd, string)
{   
    cout << "正在退出登录..." << endl;
    json js;
    js["msgid"] = QUIT_MSG;
    js["id"] = g_currentUser.getId();
    string buffer = js.dump();
    
    int len = send(clientfd, buffer.c_str(), buffer.size(), 0);
    if (len == -1)
    {
        cerr << "send quit msg error ->" << buffer << endl;
    }
    else 
    {
        isMainMenuRunning = false;
    }
    g_currentUserFriendList.clear();
    g_currentUserGroupList.clear();
    // close(clientfd);
    cout << "退出成功！" << endl;
    // exit(0);
}

void mainMenu(int clientfd)
{
    help();

    char buffer[1024] = {0};
    while (isMainMenuRunning)
    {
        cin.getline(buffer, 1024);
        string commandBuf(buffer);
        string command;     // 存储命令
        int idx = commandBuf.find(":");
        string param = "";  // 优化：定义参数变量，规避下标越界
        if (idx == -1)
        {
            command = commandBuf;
        }
        else
        {
            command = commandBuf.substr(0, idx);
            param = commandBuf.substr(idx + 1);
        }

        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            cerr << "invalid input command!" << endl;
            continue;
        }

        // 执行对应函数
        it->second(clientfd, param);
        cout << "work finish" << endl;
    }
}