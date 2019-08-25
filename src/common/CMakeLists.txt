project(common)

add_vmf_lib_test(
        common

        common.h
        ErrLogger.cpp
        ErrLogger.h
        MusicFsException.cpp
        MusicFsException.h
        ThreadPool.cpp
        ThreadPool.h
        )

target_link_libraries(common nowide)