cmake_minimum_required(VERSION 2.8)
project(dbgcli)

file(GLOB SRCS "*.cpp")
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}dap)
add_executable(dbgcli ${SRCS})
target_link_libraries(dbgcli dapcxx)
target_link_libraries(dbgcli pthread)
target_link_libraries(dbgcli ${CMAKE_EXE_LINKER_FLAGS})

