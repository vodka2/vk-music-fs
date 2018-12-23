cmake_minimum_required(VERSION 3.7)
project(vk_music_fs)

option(ENABLE_TESTS "Enable tests" OFF)

set(OLD_CXX_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_VERBOSE_MAKEFILE 1)

add_subdirectory(taglib_mod)
add_subdirectory(cfgpath)
add_subdirectory(json)
add_subdirectory(cpp-lru-cache-mod)
add_subdirectory(di)
add_subdirectory(nowide)
if(${ENABLE_TESTS})
    add_subdirectory(googletest)
endif()

set(CMAKE_CXX_FLAGS "${OLD_CXX_FLAGS} --std=gnu++17")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXX_FLAGS}")
if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
endif()

add_subdirectory(src)
if(${ENABLE_TESTS})
    add_subdirectory(tests)
endif()

add_executable(vk_music_fs main.cpp fuse_wrap.h)
target_compile_definitions(vk_music_fs PUBLIC -D_FILE_OFFSET_BITS=64)

if(DEFINED MINGW_W64)
    find_path(FUSE_INCLUDE_DIR fuse.h ${CMAKE_CURRENT_SOURCE_DIR}/winfsp/include/fuse NO_DEFAULT_PATH)
    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        set(WINFSP_NAME winfsp-x86)
    else()
        set(WINFSP_NAME winfsp-x64)
    endif()
    find_library(FUSE_LIBRARIES
        NAMES ${WINFSP_NAME}
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/winfsp ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_include_directories(vk_music_fs PUBLIC ${FUSE_INCLUDE_DIR})
else()
    find_library(FUSE_LIBRARIES
        NAMES libosxfuse.dylib fuse
        PATHS /lib64 /lib /usr/lib64 /usr/lib /usr/local/lib64 /usr/local/lib /usr/lib/x86_64-linux-gnu
    )
    message(STATUS "Found fuse ${FUSE_LIBRARIES}")
endif()

target_link_libraries(vk_music_fs vk_music_fs_lib ${FUSE_LIBRARIES})

if(APPLE)
    find_library(CoreServices CoreServices)
    target_link_libraries(vk_music_fs ${CoreServices})
endif(APPLE)
