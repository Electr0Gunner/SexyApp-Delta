add_subdirectory(zlib)
include(FetchContent)

FetchContent_Declare(
    ogg
    GIT_REPOSITORY https://github.com/teampopwork/ogg
    GIT_TAG master
)

FetchContent_Declare(
    vorbis
    GIT_REPOSITORY https://github.com/teampopwork/vorbis
    GIT_TAG master
)

FetchContent_MakeAvailable(ogg vorbis)

add_library(misc STATIC Source/stb_image.cpp)
target_include_directories(misc PUBLIC Include)