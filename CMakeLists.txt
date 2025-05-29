cmake_minimum_required(VERSION 3.20)

project(lawn C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "AMD64")
    add_compile_definitions(-D__x86_64__)
else()
    add_compile_definitions(-D__i386__)
endif()

add_subdirectory(ThirdParty)
add_subdirectory(SexyAppFramework)
add_subdirectory(Demos)
