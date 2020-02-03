cmake_minimum_required(VERSION 2.8)
project(gdbcli)

file(GLOB SRCS "*.cpp")
include_directories(.)
add_executable(gdbcli ${SRCS})
target_link_libraries(gdbcli dapcxx)
target_link_libraries(gdbcli pthread)