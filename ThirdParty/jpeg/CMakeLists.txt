set(SYSDEPMEM jmemnobs.c)

set(COMSOURCES jcomapi.c jutils.c jerror.c jmemmgr.c ${SYSDEPMEM})

set(CLIBSOURCES jcapimin.c jcapistd.c jctrans.c jcparam.c jdatadst.c
    jcinit.c jcmaster.c jcmarker.c jcmainct.c jcprepct.c
    jccoefct.c jccolor.c jcsample.c jchuff.c jcphuff.c
    jcdctmgr.c jfdctfst.c jfdctflt.c jfdctint.c
)

set(DLIBSOURCES jdapimin.c jdapistd.c jdtrans.c jdatasrc.c
    jdmaster.c jdinput.c jdmarker.c jdhuff.c jdphuff.c
    jdmainct.c jdcoefct.c jdpostct.c jddctmgr.c jidctfst.c
    jidctflt.c jidctint.c jidctred.c jdsample.c jdcolor.c
    jquant1.c jquant2.c jdmerge.c
)

set(LIBSOURCES ${CLIBSOURCES} ${DLIBSOURCES} ${COMSOURCES})
add_library(jpeg STATIC ${LIBSOURCES})

target_compile_definitions(jpeg PUBLIC -D_VISUALC_)
target_compile_definitions(jpeg PUBLIC -D_LIB)
target_include_directories(jpeg PUBLIC Include)
target_include_directories(jpeg PRIVATE Include/jpeg)
