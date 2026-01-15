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
};

// {"msgid":3,"name":"hajimi","password":"123456"}