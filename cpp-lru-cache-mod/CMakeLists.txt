project(cpp-lru-cache-mod)

add_library(cpp-lru-cache INTERFACE)
target_sources(cpp-lru-cache INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lrucache.hpp)
target_include_directories(cpp-lru-cache INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})