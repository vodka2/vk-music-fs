project(json)

add_library(json json.hpp)
target_include_directories(json PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(json PROPERTIES LINKER_LANGUAGE CXX)