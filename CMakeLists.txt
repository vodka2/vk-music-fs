cmake_minimum_required(VERSION 3.11)
project(vk_music_fs)

set(OLD_CXX_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_VERBOSE_MAKEFILE 1)

add_subdirectory(taglib_mod)

include_directories(di/extension/include)

add_subdirectory(boost-nowide)
add_subdirectory(cpp-lru-cache-mod)
add_subdirectory(googletest)

set(CMAKE_CXX_STANDARD 17)

set(${CMAKE_CXX_FLAGS} "${OLD_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXX_FLAGS}")
if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
endif()

add_subdirectory(src)
add_subdirectory(tests)

add_executable(vk_music_fs main.cpp)
target_compile_definitions(vk_music_fs PUBLIC -D_FILE_OFFSET_BITS=64)
target_link_libraries(vk_music_fs vk_music_fs_lib fuse)