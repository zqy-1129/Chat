#!/bin/bash
# 编译脚本：ChatServer 项目一键编译脚本
# 适配muduo+hiredis(stream)环境，项目根目录执行即可

# 开启调试输出 + 遇到任何错误立即退出（编译失败不再往下执行）
set -xe

# 自动创建build目录(不存在则创建)，避免rm时报错
mkdir -p build

# 清空编译缓存
rm -rf ./build/*

# 进入build编译、生成Makefile、多核编译
cd ./build && cmake .. && make -j4

# 编译成功提示
echo -e "\033[32m=====================================\033[0m"
echo -e "\033[32m✅ 编译成功! 可执行文件路径: ./bin/ChatServer\033[0m"
echo -e "\033[32m=====================================\033[0m"