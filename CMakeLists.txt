cmake_minimum_required(VERSION 3.22)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

project(SexyAppFramework)


# ----- PLATFORM SETUP -----

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

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

# ----- LIBRARY SETUP -----

set(BASS_DLL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/bass/${TARGET_ARCH}/bass.dll")
set(BASS_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/bass/${TARGET_ARCH}/bass.lib")
set(BASS_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/bass/Include")

add_subdirectory(ThirdParty)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h)
add_library(SexyAppFramework STATIC ${SOURCES})

target_include_directories(SexyAppFramework
    PUBLIC Include ${BASS_INCLUDE_PATH}
    PRIVATE Include/SexyAppFramework
)
target_link_libraries(SexyAppFramework PRIVATE misc OpenAL ogg Vorbis::vorbisfile Vorbis::vorbis zlibstatic ${BASS_LIB_PATH} winmm.lib Ws2_32.lib)

# ----- DEMO SETUP -----

option(ENABLE_DEMOS "Enable demos" ON)

if(ENABLE_DEMOS)
add_subdirectory(Demos)
endif()

# ----- TOOL SETUP -----

option(COMPILE_TOOLS "Compile tools" OFF)

if(COMPILE_TOOLS)
find_package(wxWidgets REQUIRED COMPONENTS core base net)

add_subdirectory(Tools)
endif()
