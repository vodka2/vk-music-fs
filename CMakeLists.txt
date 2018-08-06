cmake_minimum_required(VERSION 3.11)
project(vk_music_fs)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(boost-nowide)
add_subdirectory(googletest)

add_subdirectory(src)
add_subdirectory(tests)

add_executable(vk_music_fs main.cpp)
target_link_libraries(vk_music_fs vk_music_fs_lib)