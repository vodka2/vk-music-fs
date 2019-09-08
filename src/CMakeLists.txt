project(vk_music_fs)

if(DEFINED MINGW_W64)
    set(Boost_USE_STATIC_LIBS ON)
    set(OPENSSL_USE_STATIC_LIBS ON)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif()

find_package(Boost REQUIRED COMPONENTS filesystem thread program_options system)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

if(APPLE)
    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
endif()

find_package(OpenSSL REQUIRED)
if(DEFINED MINGW_W64)
    find_library(ZLIB_LIBRARY libz.a)
    set(ZLIB_LIBRARY_RESULT ${ZLIB_LIBRARY})
else()
    find_package(ZLIB REQUIRED)
    set(ZLIB_LIBRARY_RESULT ${ZLIB_LIBRARIES})
endif()

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(DEFINED MINGW_W64)
    set(EXTRA_NET_LIBS ws2_32)
endif()

add_subdirectory(common)
add_subdirectory(fs)
add_subdirectory(fuse-utils)
add_subdirectory(net)
add_subdirectory(mp3core)
add_subdirectory(main)
add_subdirectory(token)
add_subdirectory(cache)

add_executable(vk_music_fs main.cpp)

if(DEFINED MINGW_W64)
    find_path(FUSE_INCLUDE_DIR fuse.h ${PROJECT_SOURCE_DIR}/winfsp/include/fuse NO_DEFAULT_PATH)
    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        set(WINFSP_NAME winfsp-x86)
    else()
        set(WINFSP_NAME winfsp-x64)
    endif()
    find_library(FUSE_LIBRARIES
            NAMES ${WINFSP_NAME}
            PATHS ${CMAKE_CURRENT_SOURCE_DIR}/winfsp ${CMAKE_CURRENT_SOURCE_DIR}
            )
    target_include_directories(vk_music_fs PRIVATE ${FUSE_INCLUDE_DIR})
else()
    find_library(FUSE_LIBRARIES
            NAMES libosxfuse.dylib fuse
            PATHS /lib64 /lib /usr/lib64 /usr/lib /usr/local/lib64 /usr/local/lib /usr/lib/x86_64-linux-gnu
            )
endif()
message(STATUS "Found fuse ${FUSE_LIBRARIES}")

target_link_libraries(vk_music_fs ${FUSE_LIBRARIES})
target_link_libraries(vk_music_fs net mp3core fuse-utils common fs main token cache)
target_include_directories(vk_music_fs PRIVATE ${ROOT_DIR})
if(APPLE)
    find_library(CoreServices CoreServices)
    target_link_libraries(vk_music_fs ${CoreServices})
endif(APPLE)

set_target_properties(vk_music_fs PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
)