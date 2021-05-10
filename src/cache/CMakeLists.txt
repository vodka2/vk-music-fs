project(cache)

add_vmf_lib_test(cache
        File.cpp
        File.h
        FileBuffer.cpp
        FileBuffer.h
        FileCache.cpp
        FileCache.h
        PhotoCache.cpp
        PhotoCache.h
        CommonCache.cpp
        CommonCache.h
        MusicFile.cpp
        MusicFile.h
        PhotoFile.cpp
        PhotoFile.h
        CacheSaver.cpp
        CacheSaver.h
)

target_link_libraries(
        cache
        common cpp-lru-cache net json mp3core
)