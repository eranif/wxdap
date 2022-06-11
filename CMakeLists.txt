cmake_minimum_required(VERSION 3.0)
project(wxdap)

find_package(wxWidgets COMPONENTS adv base core xml xrc net stc richtext REQUIRED)
include( "${wxWidgets_USE_FILE}" )

add_subdirectory(dap)
add_subdirectory(dbgcli)
