#pragma once

#include <SizeObtainer.h>
#include <common.h>
#include <RemoteFile.h>
#include <lrucache.hpp>
#include <mutex>

namespace vk_music_fs{
    struct TotalPrepSizes{
        uint_fast32_t totalSize;
        uint_fast32_t prependSize;
    };
    class FileCache {
    public:
        FileCache(
                const std::shared_ptr<SizeObtainer> &sizeObtainer,
                SizesCacheSize sizesCacheSize,
                FilesCacheSize filesCacheSize,
                CacheDir cacheDir
        );
        FNameCache getFilename(const RemoteFile &file);
        uint_fast32_t getTagSize(const RemoteFile &file);
        uint_fast32_t getFileSize(const RemoteFile &file);
        TotalPrepSizes getInitialSize(const RemoteFile &file);
        void fileClosed(const RemoteFile &file, const TotalPrepSizes &sizes);
    private:
        std::string constructFilename(const RemoteFile &file);
        std::mutex _initialSizesMutex;
        std::mutex _sizesMutex;
        std::shared_ptr<SizeObtainer> _sizeObtainer;
        cache::lru_cache<RemoteFile, uint_fast32_t, RemoteFileHasher> _sizesCache;
        cache::lru_cache<RemoteFile, TotalPrepSizes, RemoteFileHasher> _initialSizesCache;
        std::string _cacheDir;
    };
}
