project(mp3core_tests)

add_vmf_test(FileProcessorTest mp3core FileProcessorT.cpp)
add_vmf_test(PhotoProcessorTest mp3core PhotoProcessorT.cpp)
add_vmf_test(Mp3ParserTest mp3core Mp3ParserT.cpp)
add_vmf_test(FileProcessorMp3ParserTest mp3core FileProcessorMp3ParserT.cpp)
add_vmf_test(FileManagerTest mp3core FileManagerT.cpp)
add_vmf_test(BlockingBufferTest mp3core BlockingBufferT.cpp)