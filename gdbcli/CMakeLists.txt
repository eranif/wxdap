cmake_minimum_required(VERSION 2.8)
project(dbgcli)

file(GLOB SRCS "*.cpp")
include_directories(.)
add_executable(dbgcli ${SRCS})
target_link_libraries(dbgcli dapcxx)
target_link_libraries(dbgcli pthread)