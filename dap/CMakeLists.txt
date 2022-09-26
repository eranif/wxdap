cmake_minimum_required(VERSION 3.0)
project(dapcxx)

include_directories(${CMAKE_SOURCE_DIR})
FILE(GLOB SRCS "*.cpp")

find_package(wxWidgets COMPONENTS adv aui base core xml xrc net stc richtext REQUIRED)
include( "${wxWidgets_USE_FILE}" )

set( ADDITIONAL_LIBRARIES "" )

if(MINGW)
set(ADDITIONAL_LIBRARIES "-lws2_32")
endif()

add_library(dapcxx SHARED ${SRCS})
target_link_libraries(dapcxx ${wxWidgets_LIBRARIES} ${ADDITIONAL_LIBRARIES})
