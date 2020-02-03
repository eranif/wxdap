cmake_minimum_required(VERSION 2.8)
project(dbgd)
add_subdirectory(dap)
add_subdirectory(gdbcli)

file(GLOB SRCS "*.cpp")
include_directories(.)
add_executable(dbgd ${SRCS})

target_link_libraries(dbgd dapcxx)
target_link_libraries(dbgd pthread)

