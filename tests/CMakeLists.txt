cmake_minimum_required(VERSION 3.10)
project(daptests)

include_directories(${CMAKE_SOURCE_DIR})
FILE(GLOB SRCS "*.cpp")

add_executable(daptests ${SRCS})
target_link_libraries(daptests dapcxx)
add_test(NAME tests COMMAND daptests)
