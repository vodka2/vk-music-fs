project(common)

add_vmf_lib_test(
        common

        common.h
        IOBlock.h
        IOBlockCreator.h
        ErrLogger.cpp
        ErrLogger.h
        MusicFsException.cpp
        MusicFsException.h
        ThreadPool.cpp
        ThreadPool.h
        RealFs.cpp
        RealFs.h
        )

target_link_libraries(common nowide)