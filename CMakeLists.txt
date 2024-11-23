cmake_minimum_required(VERSION 3.5)
project(wxdap)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(wxWidgets COMPONENTS adv base core xml xrc net stc richtext REQUIRED)
include( "${wxWidgets_USE_FILE}" )

add_subdirectory(dap)
add_subdirectory(dbgcli)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
