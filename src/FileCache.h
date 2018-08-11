#pragma once

#include <common.h>
#include <SizeObtainer.h>
#include <RemoteFile.h>
#include <lrucache.hpp>

namespace vk_music_fs{
    class FileCache {
    public:
        FileCache(const std::shared_ptr<SizeObtainer> &sizeObtainer, SizesCacheSize sizesCacheSize, FilesCacheSize filesCacheSize);
        std::string getFilename(const RemoteFile &file);
        uint_fast32_t getTagSize(const RemoteFile &file);
        uint_fast32_t getFileSize(const RemoteFile &file);
        void fileClosed(const std::string &fname);
    private:
        struct FileCacheItem{
            std::string name;
            ~FileCacheItem(){
                std::remove(name.c_str());
            }
        };
        std::shared_ptr<SizeObtainer> _sizeObtainer;
        cache::lru_cache<RemoteFile, uint_fast32_t, RemoteFileHasher> _sizesCache;
        cache::lru_cache<RemoteFile, FileCacheItem, RemoteFileHasher> _filesCache;
    };
}
