project(cfgpath)

add_library(cfgpath INTERFACE)
target_sources(cfgpath INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cfgpath.h)
target_include_directories(cfgpath INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/boost-nowide)