project(fuse_utils)

add_library(fuse-utils INTERFACE)
target_sources(fuse-utils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fuse_wrap.h)
target_compile_definitions(fuse-utils INTERFACE -D_FILE_OFFSET_BITS=64)