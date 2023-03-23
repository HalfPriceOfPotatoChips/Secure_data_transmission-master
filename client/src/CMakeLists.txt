aux_source_directory(. SRC_LIST)
# 生成库文件
add_library(secclient SHARED ${SRC_LIST})
# 链接库文件
target_link_libraries(secclient pthread crypto jsoncpp)