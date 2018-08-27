project(vk_music_fs_lib)

set(LIB_SRCS FileProcessor.cpp FileProcessor.h common.h Mp3Parser.cpp Mp3Parser.h BlockingBuffer.cpp BlockingBuffer.h net/HttpStream.h net/HttpStream.cpp net/HttpException.cpp net/HttpException.h net/HttpStreamCommon.cpp net/HttpStreamCommon.h MusicFile.cpp MusicFile.h ThreadPool.cpp ThreadPool.h net/SizeObtainer.cpp net/SizeObtainer.h RemoteFile.cpp RemoteFile.h FileManager.cpp FileManager.h Reader.cpp Reader.h FileCache.cpp FileCache.h ext_factory.hpp net/VkApiQueryMaker.cpp net/VkApiQueryMaker.h fs/AudioFs.cpp fs/AudioFs.h Application.cpp Application.h RemoteException.cpp RemoteException.h fs/Dir.cpp fs/Dir.h fs/common_fs.h fs/File.cpp fs/File.h fs/DirOrFile.cpp fs/DirOrFile.h fs/OffsetCntName.cpp fs/OffsetCntName.h fs/OffsetCnt.cpp fs/OffsetCnt.h fs/SearchDirMaker.cpp fs/SearchDirMaker.h ProgramOptions.cpp ProgramOptions.h fs/VkException.cpp fs/VkException.h fs/FsException.cpp fs/FsException.h MusicFsException.cpp MusicFsException.h ErrLogger.cpp ErrLogger.h)

if(DEFINED MINGW_W64)
    set(LIB_SRCS ${LIB_SRCS} ${CMAKE_SOURCE_DIR}/boost-nowide/libs/nowide/src/iostream.cpp)
endif()

add_library(vk_music_fs_lib ${LIB_SRCS})

target_include_directories(vk_music_fs_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/di/include ${CMAKE_SOURCE_DIR}/di/extension/include ${CMAKE_SOURCE_DIR}/boost-nowide)

if(DEFINED MINGW_W64)
    target_link_libraries(vk_music_fs_lib json cfgpath cpp-lru-cache tag z boost_system-mt boost_filesystem-mt boost_thread-mt boost_program_options-mt pthread ssl crypto ws2_32)
else()
    target_link_libraries(vk_music_fs_lib json cfgpath cpp-lru-cache tag z boost_system boost_filesystem boost_thread boost_program_options pthread ssl crypto)
endif()