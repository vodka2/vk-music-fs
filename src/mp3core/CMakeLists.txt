project(mp3core)

add_vmf_lib_test(mp3core
        common_mp3core.h
        BlockingBuffer.cpp
        BlockingBuffer.h
        FileManager.cpp
        FileManager.h
        FileProcessor.cpp
        FileProcessor.h
        FileProcessorCreator.cpp
        FileProcessorCreator.h
        PhotoProcessor.cpp
        PhotoProcessor.h
        PhotoProcessorCreator.cpp
        PhotoProcessorCreator.h
        Mp3Parser.cpp
        Mp3Parser.h
        Reader.cpp
        Reader.h
        RemoteException.cpp
        RemoteException.h
        RemoteFile.cpp
        RemoteFile.h
        TagSizeCalculator.cpp
        TagSizeCalculator.h
)

target_link_libraries(
        mp3core
        common json
        tag ${ZLIB_LIBRARY_RESULT} cpp-lru-cache di
)

add_subdirectory(0tests)