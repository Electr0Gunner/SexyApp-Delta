file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h)
add_library(SexyAppFramework STATIC ${SOURCES})

target_include_directories(SexyAppFramework
    PUBLIC Include ${BASS_INCLUDE_PATH}
    PRIVATE Include/SexyAppFramework
)
target_link_libraries(SexyAppFramework PRIVATE misc ogg Vorbis::vorbisfile Vorbis::vorbis ${BASS_LIB_PATH} winmm.lib Ws2_32.lib)
