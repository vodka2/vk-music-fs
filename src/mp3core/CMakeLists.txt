project(mp3core)

add_vmf_lib_test(mp3core
        BlockingBuffer.cpp
        BlockingBuffer.h
        FileManager.cpp
        FileManager.h
        FileProcessor.cpp
        FileProcessor.h
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