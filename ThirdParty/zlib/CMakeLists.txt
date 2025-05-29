set(SOURCES adler32.c compress.c crc32.c gzio.c uncompr.c deflate.c trees.c
    zutil.c inflate.c infblock.c inftrees.c infcodes.c infutil.c inffast.c
)

add_library(zlib STATIC ${SOURCES})

target_include_directories(zlib PUBLIC Include)
target_include_directories(zlib PRIVATE Include/zlib)
