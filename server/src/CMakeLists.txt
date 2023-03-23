aux_source_directory(. SRC_LIST)
# 生成库文件
add_library(mysecure SHARED ${SRC_LIST})
# 链接库文件
target_link_libraries(mysecure pthread crypto mysqlclient jsoncpp)