cmake_minimum_required(VERSION 3.11)
project(vk_music_fs)

add_subdirectory(boost-nowide)

set(CMAKE_CXX_STANDARD 17)

add_executable(vk_music_fs src/main.cpp src/FileProcessor.cpp src/FileProcessor.h)