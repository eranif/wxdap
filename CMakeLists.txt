cmake_minimum_required(VERSION 2.8)
project(gdbd)
add_subdirectory(dap)

file(GLOB SRCS "*.cpp")
include_directories(.)
add_executable(gdbd ${SRCS})

target_link_libraries(gdbd dapcxx)
target_link_libraries(gdbd pthread)

