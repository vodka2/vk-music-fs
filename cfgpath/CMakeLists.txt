project(cfgpath)

add_library(cfgpath cfgpath.h)
target_include_directories(cfgpath PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(cfgpath PROPERTIES LINKER_LANGUAGE CXX)