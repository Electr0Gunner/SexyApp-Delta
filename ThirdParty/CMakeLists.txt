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

FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib
    GIT_TAG master
)

FetchContent_Declare(
    openal
    GIT_REPOSITORY https://github.com/kcat/openal-soft
    GIT_TAG master
)

FetchContent_Declare(
    sdl
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    GIT_TAG main
)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG master
)

set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(ogg vorbis zlib openal sdl glm)

add_library(ThirdParty STATIC Source/stb_image.cpp)
target_link_libraries(ThirdParty PUBLIC glm SDL3::SDL3-static OpenAL ogg Vorbis::vorbisfile Vorbis::vorbis zlibstatic )
target_include_directories(ThirdParty PUBLIC Include)