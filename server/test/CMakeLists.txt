set(SRC_LIST transit_data.pb.cc)
add_executable(test ${SRC_LIST} main.cpp)
target_link_libraries(test mysecure protobuf pthread crypto mysqlclient jsoncpp)