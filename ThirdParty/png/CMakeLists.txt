add_library(png STATIC png.c pngerror.c pngget.c pngmem.c pngread.c pngrio.c pngrtran.c pngrutil.c pngset.c pngtrans.c pngvcrd.c pngwio.c pngwrite.c pngwtran.c pngwutil.c)

target_link_libraries(png PUBLIC zlib)
target_include_directories(png PUBLIC Include)
target_include_directories(png PRIVATE Include/png)
