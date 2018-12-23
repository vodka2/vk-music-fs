project(json)

add_library(json INTERFACE)
target_sources(json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/json.hpp)
target_include_directories(json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})