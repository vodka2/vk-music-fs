project(vk_music_fs_lib)

add_library(vk_music_fs_lib FileProcessor.cpp FileProcessor.h common.h Mp3Parser.cpp Mp3Parser.h BlockingBuffer.cpp BlockingBuffer.h)

target_include_directories(vk_music_fs_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/di/include)

if(DEFINED MINGW_W64)
    target_link_libraries(vk_music_fs_lib tag z boost_system-mt boost_thread-mt pthread ssl crypto ws2_32)
else()
    target_link_libraries(vk_music_fs_lib tag z boost_system boost_thread pthread ssl crypto)
endif()