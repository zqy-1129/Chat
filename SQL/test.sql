-- ==============================================
-- 测试数据初始化脚本
-- 测试用户账号：用户名=密码，状态默认离线
-- ==============================================
USE chatdb;

-- 插入测试用户
INSERT INTO User(name, password, state) VALUES('张三', '123456', 'offline');
INSERT INTO User(name, password, state) VALUES('李四', '654321', 'offline');
INSERT INTO User(name, password, state) VALUES('哈基米', '654321', 'offline');

-- 插入测试离线消息（给用户id=1的张三）
INSERT INTO OfflineMessage(userId, message) VALUES(1, "{\"msgid\":1,\"from\":2,\"to\":1,\"msg\":\"你好，我是李四\",\"time\":\"2026-01-15 12:00:00\"}");