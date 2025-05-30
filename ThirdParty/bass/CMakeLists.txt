add_library(bass INTERFACE)
target_include_directories(bass INTERFACE Include)
set_property(TARGET bass PROPERTY IMPORTED_LOCATION "${TARGET_ARCH}/bass.lib")
