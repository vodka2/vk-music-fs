project(vk_music_fs_lib)

set(LIB_SRCS FileProcessor.cpp FileProcessor.h common.h Mp3Parser.cpp Mp3Parser.h BlockingBuffer.cpp BlockingBuffer.h net/HttpStream.h net/HttpStream.cpp net/HttpException.cpp net/HttpException.h net/HttpStreamCommon.cpp net/HttpStreamCommon.h MusicFile.cpp MusicFile.h ThreadPool.cpp ThreadPool.h net/SizeObtainer.cpp net/SizeObtainer.h RemoteFile.cpp RemoteFile.h FileManager.cpp FileManager.h Reader.cpp Reader.h FileCache.cpp FileCache.h ext_factory.hpp net/VkApiQueryMaker.cpp net/VkApiQueryMaker.h fs/AudioFs.cpp fs/AudioFs.h Application.cpp Application.h RemoteException.cpp RemoteException.h fs/Dir.cpp fs/Dir.h fs/common_fs.h fs/File.cpp fs/File.h fs/DirOrFile.cpp fs/DirOrFile.h fs/OffsetCntName.cpp fs/OffsetCntName.h fs/OffsetCnt.cpp fs/OffsetCnt.h fs/SearchDirMaker.cpp fs/SearchDirMaker.h ProgramOptions.cpp ProgramOptions.h fs/VkException.cpp fs/VkException.h fs/FsException.cpp fs/FsException.h MusicFsException.cpp MusicFsException.h ErrLogger.cpp ErrLogger.h fs/FileName.cpp fs/FileName.h token/TokenReceiver.cpp token/TokenReceiver.h token/SmallProtobufHelper.cpp token/SmallProtobufHelper.h token/common_token.h token/ProtobufException.cpp token/ProtobufException.h token/TokenException.cpp token/TokenException.h)

if(DEFINED MINGW_W64)
    set(Boost_USE_STATIC_LIBS ON)
    set(OPENSSL_USE_STATIC_LIBS ON)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif()

add_library(vk_music_fs_lib ${LIB_SRCS})

target_include_directories(vk_music_fs_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Boost REQUIRED COMPONENTS filesystem thread program_options system)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_library(ZLIB_LIBRARY libz.a)

set(MAIN_LIBS json cfgpath cpp-lru-cache tag di nowide ${ZLIB_LIBRARY} ${Boost_LIBRARIES} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)

if(DEFINED MINGW_W64)
    list(APPEND MAIN_LIBS ws2_32)
endif()
target_link_libraries(vk_music_fs_lib ${MAIN_LIBS})