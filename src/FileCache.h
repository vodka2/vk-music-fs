#pragma once

#include <SizeObtainer.h>
#include <common.h>
#include <RemoteFile.h>
#include <lrucache.hpp>
#include <mutex>

namespace vk_music_fs{
    class FileCache {
    public:
        FileCache(const std::shared_ptr<SizeObtainer> &sizeObtainer, SizesCacheSize sizesCacheSize, FilesCacheSize filesCacheSize);
        FNameCache getFilename(const RemoteFile &file);
        uint_fast32_t getTagSize(const RemoteFile &file);
        uint_fast32_t getFileSize(const RemoteFile &file);
        uint_fast32_t getInitialSize(const RemoteFile &file);
        void fileClosed(const RemoteFile &file, uint_fast32_t curSize);
    private:
        std::string constructFilename(const RemoteFile &file);
        std::mutex _initialSizesMutex;
        std::mutex _sizesMutex;
        std::shared_ptr<SizeObtainer> _sizeObtainer;
        cache::lru_cache<RemoteFile, uint_fast32_t, RemoteFileHasher> _sizesCache;
        cache::lru_cache<RemoteFile, uint_fast32_t, RemoteFileHasher> _initialSizesCache;
    };
}
