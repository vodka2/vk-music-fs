project(vk_music_fs_tests)

add_library(testlib data/Mp3Files.h data/FileM.h data/ThreadPoolM.h data/StreamM.h data/MusicData.h data/Writer.h)
target_link_libraries(testlib PUBLIC gtest_main gmock)
target_include_directories(testlib PUBLIC ${CURRENT_SRC_DIR})

function(add_vmf_test name src)
    add_executable(${name} "${src}")
    target_link_libraries(${name} PUBLIC testlib)
    target_link_libraries(${name} PUBLIC vk_music_fs_lib)
endfunction()

add_vmf_test(FileProcessorTest FileProcessorT.cpp)
add_vmf_test(Mp3ParserTest Mp3ParserT.cpp)
add_vmf_test(FileProcessorMp3ParserTest FileProcessorMp3ParserT.cpp)
add_vmf_test(FileManagerTest FileManagerT.cpp)
add_vmf_test(AudioFsTest AudioFsT.cpp)