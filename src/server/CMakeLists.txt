# 所有源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./database DB_LIST)
aux_source_directory(./model MODEL_LIST)

# 可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST})

# 链接库文件
target_link_libraries(ChatServer muduo_base muduo_net pthread mysqlclient)