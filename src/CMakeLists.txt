project(vk_music_fs_lib)

add_library(vk_music_fs_lib FileProcessor.cpp FileProcessor.h common.h Mp3Parser.cpp Mp3Parser.h BlockingBuffer.cpp BlockingBuffer.h)

target_include_directories(vk_music_fs_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/di/include)

target_link_libraries(vk_music_fs_lib boost_system boost_thread tag z)