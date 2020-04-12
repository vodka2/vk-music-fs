project(fs_tests)

add_vmf_test(AudioFsTest fs AudioFsT.cpp)
add_vmf_test(FileNameTest fs FileNameT.cpp)
add_vmf_test(FsUtilsTest fs FsUtilsT.cpp)
add_vmf_test(PlaylistCtrlTest fs PlaylistCtrlT.cpp)
add_vmf_test(SimilarCtrlTest fs SimilarCtrlT.cpp)
add_vmf_test(AsyncFsManagerTest fs AsyncFsManagerT.cpp)