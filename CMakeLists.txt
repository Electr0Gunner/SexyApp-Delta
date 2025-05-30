cmake_minimum_required(VERSION 3.20)

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
    set(BASS_PATH "${CMAKE_SOURCE_DIR}/ThirdParty/bass/X86/bass.dll")
    add_compile_definitions(-D__i386__)
elseif(TARGET_ARCH STREQUAL "X64")
    set(BASS_PATH "${CMAKE_SOURCE_DIR}/ThirdParty/bass/X64/bass.dll")
    add_compile_definitions(-D__x86_64__)
else()
    message(FATAL_ERROR "TARGET_ARCH not set (this is a bug in CMakeLists.txts.")
endif()

add_subdirectory(ThirdParty)
add_subdirectory(SexyAppFramework)
add_subdirectory(Demos)
