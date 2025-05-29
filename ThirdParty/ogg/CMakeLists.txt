add_library(ogg STATIC bitwise.c block.c codebook.c floor0.c floor1.c framing.c info.c mapping0.c mdct.c registry.c res012.c sharedbook.c synthesis.c vorbisfile.c window.c)

target_include_directories(ogg PUBLIC Include)
target_include_directories(ogg PRIVATE Include/ogg)
