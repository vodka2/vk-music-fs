project(vk_music_fs_tests)

add_library(testlib INTERFACE)
target_link_libraries(testlib INTERFACE gtest_main gmock)
target_include_directories(testlib INTERFACE ${CURRENT_SRC_DIR})

function(add_vmf_test name src)
    add_executable(${name} "${src}")
    target_link_libraries(${name} PUBLIC testlib)
    target_link_libraries(${name} PUBLIC vk_music_fs_lib)
endfunction()

add_vmf_test(FileProcessorTest FileProcessorT.cpp)