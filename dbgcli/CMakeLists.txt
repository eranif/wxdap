cmake_minimum_required(VERSION 3.0)
project(dap_demo)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_package(wxWidgets COMPONENTS adv base core xml xrc net stc richtext REQUIRED)
include( "${wxWidgets_USE_FILE}" )

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
endif()

include_directories(${CMAKE_SOURCE_DIR})
FILE(GLOB SRCS "*.cpp")

add_executable(dap_demo ${SRCS})
target_link_libraries(dap_demo dapcxx ${wxWidgets_LIBRARIES})
