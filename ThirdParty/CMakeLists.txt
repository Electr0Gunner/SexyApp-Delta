add_subdirectory(jpeg)
add_subdirectory(zlib)
add_subdirectory(png)

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