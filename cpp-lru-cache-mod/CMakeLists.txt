project(cpp-lru-cache-mod)

add_library(cpp-lru-cache lrucache.hpp)
target_include_directories(cpp-lru-cache PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(cpp-lru-cache PROPERTIES LINKER_LANGUAGE CXX)