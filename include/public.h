#pragma once

/**
 * server和client的公用文件
 */

enum EnMsgType
{
    LOGIN_MSG = 1,    // 登录消息
    LOGIN_MSG_ACK,    // 登录响应消息
    REG_MSG,          // 注册消息
    REG_MSG_ACK,      // 注册响应消息
    ONE_CHAT_MSG,     // 聊天消息
    ADD_FRIEND_MSG,   // 添加好友消息

    CREATE_GROUP_MSG, // 创建群组
    ADD_GROUP_MSG,    // 加入群组
    GROUP_CHAT_MSG,   // 群聊天
};

// {"msgid":3,"name":"hajimi","password":"123456"}
// {"msgid":1,"id":2,"password":"654321"}
// {"msgid":1,"id":1,"password":"123456"}
// {"msgid":5,"id":2,"from":"哈基米","to":1,"msg":"咪"}
// {"msgid":5,"id":2,"from":"哈基米","to":1,"msg":"咪咪"}
// {"msgid":6,"id":2,"friendId":1}