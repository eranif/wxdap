cmake_minimum_required(VERSION 3.0)
project(dapcxx)

include_directories(${CMAKE_SOURCE_DIR})
FILE(GLOB SRCS "*.cpp")

# find wxWidgets once
if (MINGW)
    if (NOT wxWidgets_USE_FILE)
        if (NOT WXWIN)
            message(FATAL_ERROR "Missing -DWXWIN=<wxWidgets/root/install/dir>")
        endif()
        if (NOT WXCFG)
            set(WXCFG "clang_x64_dll/mswu")
        endif()

        message(STATUS "WXWIN is set to ${WXWIN}")
        message(STATUS "WXCFG is set to ${WXCFG}")
        
        execute_process(COMMAND
            ${CL_WX_CONFIG} --cmake --prefix=${WXWIN} --wxcfg=${WXCFG} 
            OUTPUT_VARIABLE wxWidgets_USE_FILE
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        message(STATUS "wxWidgets_USE_FILE is set to: ${wxWidgets_USE_FILE}")
    endif()
else()
    find_package(wxWidgets COMPONENTS adv aui base core html propgrid xml xrc net stc ribbon richtext REQUIRED)
endif()

include( "${wxWidgets_USE_FILE}" )

set( ADDITIONAL_LIBRARIES "" )

if(MINGW)
set(ADDITIONAL_LIBRARIES "-lws2_32")
endif()

add_library(dapcxx SHARED ${SRCS})
target_compile_definitions(dapcxx PRIVATE WXMAKINGDLL_DAP)
target_compile_definitions(dapcxx INTERFACE WXUSINGDLL_DAP)
target_link_libraries(dapcxx ${wxWidgets_LIBRARIES} ${ADDITIONAL_LIBRARIES})
