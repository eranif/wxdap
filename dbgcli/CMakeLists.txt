cmake_minimum_required(VERSION 3.5)
project(dap_demo)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_package(wxWidgets COMPONENTS adv base core xml xrc net stc richtext REQUIRED)
include( "${wxWidgets_USE_FILE}" )

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
endif()

include_directories(${CMAKE_SOURCE_DIR})
FILE(GLOB SRCS "*.cpp")

add_executable(dap_demo WIN32 ${SRCS})
target_link_libraries(dap_demo dapcxx ${wxWidgets_LIBRARIES})
set_property(TARGET dapcxx PROPERTY CXX_STANDARD 17)
set_property(TARGET dapcxx PROPERTY CXX_EXTENSIONS OFF)
STRING(REPLACE "-std=c++11" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

