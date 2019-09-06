project(cache)

add_vmf_lib_test(cache
        File.cpp
        File.h
        FileBuffer.cpp
        FileBuffer.h
        FileCache.cpp
        FileCache.h
        MusicFile.cpp
        MusicFile.h
        CacheSaver.cpp
        CacheSaver.h
)

target_link_libraries(
        cache
        common cpp-lru-cache net json mp3core
)