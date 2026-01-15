-- ==============================================
-- Chat聊天服务器 完整数据库初始化脚本
-- 包含：User、Friend、AllGroup、GroupUser、OfflineMessage 所有业务表
-- 编码：UTF8 解决中文乱码
-- ==============================================

-- 1. 创建并使用数据库
CREATE DATABASE IF NOT EXISTS chatdb CHARACTER SET utf8 COLLATE utf8_general_ci;
USE chatdb;

-- ==============================================
-- 1. 用户表 User
-- 存储用户账号、密码、在线状态
-- ==============================================
DROP TABLE IF EXISTS User;
CREATE TABLE User (
    id INT NOT NULL AUTO_INCREMENT COMMENT '用户id，主键自增',
    name VARCHAR(50) NOT NULL COMMENT '用户名，唯一不可重复',
    password VARCHAR(50) NOT NULL COMMENT '用户密码',
    state ENUM('online','offline') DEFAULT 'offline' COMMENT '用户状态：在线/离线',
    PRIMARY KEY (id),
    UNIQUE KEY uk_name (name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='用户信息表';

-- ==============================================
-- 2. 好友关系表 Friend
-- 存储用户的好友列表（联合主键保证关系唯一）
-- ==============================================
DROP TABLE IF EXISTS Friend;
CREATE TABLE Friend (
    userid INT NOT NULL COMMENT '用户id',
    friendid INT NOT NULL COMMENT '好友id',
    PRIMARY KEY (userid, friendid)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='好友关系表';

-- ==============================================
-- 3. 群组信息表 AllGroup
-- 存储所有群组的基础信息
-- ==============================================
DROP TABLE IF EXISTS AllGroup;
CREATE TABLE AllGroup (
    id INT NOT NULL AUTO_INCREMENT COMMENT '群id，主键自增',
    groupname VARCHAR(50) NOT NULL COMMENT '群名称',
    groupdesc VARCHAR(200) DEFAULT '' COMMENT '群描述',
    PRIMARY KEY (id),
    UNIQUE KEY uk_groupname (groupname)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='群组信息表';

-- ==============================================
-- 4. 群成员表 GroupUser
-- 存储用户在群组中的角色
-- ==============================================
DROP TABLE IF EXISTS GroupUser;
CREATE TABLE GroupUser (
    groupid INT NOT NULL COMMENT '群id',
    userid INT NOT NULL COMMENT '用户id',
    grouprole ENUM('creator','normal') DEFAULT 'normal' COMMENT '群内角色：创建者/普通成员',
    PRIMARY KEY (groupid, userid)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='群成员信息表';

-- ==============================================
-- 5. 离线消息表 OfflineMessage
-- 存储用户离线时收到的消息（支持多消息存储的最优结构）
-- ==============================================
DROP TABLE IF EXISTS OfflineMessage;
CREATE TABLE OfflineMessage (
    id INT NOT NULL AUTO_INCREMENT COMMENT '消息id，主键自增',
    userId INT NOT NULL COMMENT '接收消息的用户id',
    message VARCHAR(500) NOT NULL COMMENT '离线消息内容（JSON格式）',
    PRIMARY KEY (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='用户离线消息表';