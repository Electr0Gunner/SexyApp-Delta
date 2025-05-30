cmake_minimum_required(VERSION 3.30)

project(lawn C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

# Detect target architecture for MSVC
if(CMAKE_C_COMPILER_ARCHITECTURE_ID STREQUAL "X86")
    set(TARGET_ARCH "X86")
elseif(CMAKE_C_COMPILER_ARCHITECTURE_ID STREQUAL "x64")
    set(TARGET_ARCH "X64")
# Target architecture could not be determined
else()
    message(FATAL_ERROR "Unsupported architecture: ${CMAKE_C_COMPILER_ARCHITECTURE_ID}.")
endif()

if (TARGET_ARCH STREQUAL "X86")
    add_compile_definitions(-D__i386__)
elseif(TARGET_ARCH STREQUAL "X64")
    add_compile_definitions(-D__x86_64__)
else()
    message(FATAL_ERROR "TARGET_ARCH not set (this is a bug in CMakeLists.txts.")
endif()

set(BASS_DLL_PATH "${CMAKE_SOURCE_DIR}/ThirdParty/bass/${TARGET_ARCH}/bass.dll")
set(BASS_LIB_PATH "${CMAKE_SOURCE_DIR}/ThirdParty/bass/${TARGET_ARCH}/bass.lib")
set(BASS_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/ThirdParty/bass/Include")

option(ENABLE_DEMOS "Enable demos" ON)

add_subdirectory(ThirdParty)
add_subdirectory(SexyAppFramework)

if(ENABLE_DEMOS)
add_subdirectory(Demos)
endif()

