project(vk_music_fs_lib)

set(LIB_SRCS FileProcessor.cpp FileProcessor.h common.h Mp3Parser.cpp Mp3Parser.h BlockingBuffer.cpp BlockingBuffer.h net/HttpStream.h net/HttpStream.cpp net/HttpException.cpp net/HttpException.h net/HttpStreamCommon.cpp net/HttpStreamCommon.h MusicFile.cpp MusicFile.h ThreadPool.cpp ThreadPool.h net/Mp3SizeObtainer.cpp net/Mp3SizeObtainer.h RemoteFile.cpp RemoteFile.h FileManager.cpp FileManager.h Reader.cpp Reader.h FileCache.cpp FileCache.h diext/ext_factory.hpp net/VkApiQueryMaker.cpp net/VkApiQueryMaker.h fs/AudioFs.cpp fs/AudioFs.h Application.cpp Application.h RemoteException.cpp RemoteException.h fs/Dir.cpp fs/Dir.h fs/common_fs.h fs/File.cpp fs/File.h fs/DirOrFile.cpp fs/DirOrFile.h fs/OffsetCntName.cpp fs/OffsetCntName.h fs/OffsetCnt.cpp fs/OffsetCnt.h ProgramOptions.cpp ProgramOptions.h fs/VkException.cpp fs/VkException.h fs/FsException.cpp fs/FsException.h MusicFsException.cpp MusicFsException.h ErrLogger.cpp ErrLogger.h fs/FileName.cpp fs/FileName.h token/TokenReceiver.cpp token/TokenReceiver.h token/SmallProtobufHelper.cpp token/SmallProtobufHelper.h token/common_token.h token/ProtobufException.cpp token/ProtobufException.h token/TokenException.cpp token/TokenException.h TagSizeCalculator.cpp TagSizeCalculator.h net/WrongSizeException.cpp net/WrongSizeException.h CacheSaver.cpp CacheSaver.h fs/FsUtils.cpp fs/FsUtils.h fs/FsPath.cpp fs/FsPath.h fs/ctrl/MyAudiosCtrl.cpp fs/ctrl/MyAudiosCtrl.h fs/FsSettings.cpp fs/FsSettings.h fs/IdGenerator.cpp fs/IdGenerator.h fs/FileObtainer.cpp fs/FileObtainer.h fs/ctrl/DummyDirWrapper.cpp fs/ctrl/DummyDirWrapper.h fs/ctrl/RootCtrl.cpp fs/ctrl/RootCtrl.h fs/ctrl/RemoteFileWrapper.cpp fs/ctrl/RemoteFileWrapper.h fs/ctrl/ThrowExCtrl.cpp fs/ctrl/ThrowExCtrl.h fs/ctrl/RedirectCtrl.cpp fs/ctrl/RedirectCtrl.h fs/ctrl/SearchSongNameCtrl.cpp fs/ctrl/SearchSongNameCtrl.h fs/ctrl/SingleDirCtrl.cpp fs/ctrl/SingleDirCtrl.h fs/ctrl/SearchSongNameSongHelper.cpp fs/ctrl/SearchSongNameSongHelper.h fs/ctrl/SearchSongNameArtistHelper.cpp fs/ctrl/SearchSongNameArtistHelper.h fs/actions/RefreshAct.cpp fs/actions/RefreshAct.h fs/actions/act.h fs/actions/NumberAct.cpp fs/actions/NumberAct.h diext/goodscoped.h diext/common_di.h diext/Storage.h)

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

if(APPLE)
    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
endif()

find_package(OpenSSL REQUIRED)
if(DEFINED MINGW_W64)
    find_library(ZLIB_LIBRARY libz.a)
    set(ZLIB_LIBRARY_RESULT ${ZLIB_LIBRARY})
else()
    find_package(ZLIB REQUIRED)
    set(ZLIB_LIBRARY_RESULT ${ZLIB_LIBRARIES})
endif()

set(
    MAIN_LIBS json cfgpath cpp-lru-cache tag di nowide ${ZLIB_LIBRARY_RESULT}
    Boost::filesystem Boost::thread Boost::program_options Boost::system Threads::Threads OpenSSL::SSL OpenSSL::Crypto
)

if(DEFINED MINGW_W64)
    list(APPEND MAIN_LIBS ws2_32)
endif()
target_link_libraries(vk_music_fs_lib ${MAIN_LIBS})