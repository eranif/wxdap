cmake_minimum_required(VERSION 3.0)
project(dapcxx)
include_directories(${CMAKE_SOURCE_DIR})
FILE(GLOB SRCS "*.cpp")

add_library(dapcxx STATIC ${SRCS})
