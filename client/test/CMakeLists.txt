set(SRC_LIST transit_data.pb.cc)
add_executable(clienttest ${SRC_LIST} main.cpp)
target_link_libraries(clienttest secclient protobuf pthread crypto jsoncpp)