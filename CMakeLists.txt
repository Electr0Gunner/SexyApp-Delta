cmake_minimum_required(VERSION 3.20)

project(lawn C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(ThirdParty)
add_subdirectory(SexyAppFramework)
add_subdirectory(Demos)
