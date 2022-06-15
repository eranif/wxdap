cmake_minimum_required(VERSION 3.0)
project(dapcxx)
include_directories(${CMAKE_SOURCE_DIR})
FILE(GLOB SRCS "*.cpp")

find_package(wxWidgets COMPONENTS adv aui base core xml xrc net stc richtext REQUIRED)

add_library(dapcxx SHARED ${SRCS})
target_link_libraries(dapcxx ${wxWidgets_LIBRARIES})
