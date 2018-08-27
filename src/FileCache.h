#pragma once

#include "net/SizeObtainer.h"
#include <common.h>
#include <RemoteFile.h>
#include <lrucache.hpp>
#include <mutex>
#include <unordered_set>

namespace vk_music_fs{
    struct TotalPrepSizes{
        uint_fast32_t totalSize;
        uint_fast32_t prependSize;
    };
    class FileCache {
    public:
        FileCache(
                const std::shared_ptr<net::SizeObtainer> &sizeObtainer,
                SizesCacheSize sizesCacheSize,
                FilesCacheSize filesCacheSize,
                CacheDir cacheDir
        );
        ~FileCache();
        FNameCache getFilename(const RemoteFile &file);
        uint_fast32_t getTagSize(const RemoteFile &file);
        uint_fast32_t getFileSize(const RemoteFile &file);
        TotalPrepSizes getInitialSize(const RemoteFileId &file);
        void fileClosed(const RemoteFile &file, const TotalPrepSizes &sizes);
    private:
        constexpr static std::string_view SIZES_CACHE_FNAME = "head_cache.json";
        constexpr static std::string_view INITIAL_SIZES_CACHE_FNAME = "files_cache.json";
        std::string idToStr(const RemoteFileId &file);
        void loadSizesFromFile();
        void saveSizesToFile();
        void loadInitialSizesFromFile();
        void saveInitialSizesToFile();
        std::string constructFilename(const RemoteFileId &file);
        std::mutex _initialSizesMutex;
        std::mutex _sizesMutex;
        std::shared_ptr<net::SizeObtainer> _sizeObtainer;
        cache::lru_cache<RemoteFileId, uint_fast32_t, RemoteFileIdHasher> _sizesCache;
        cache::lru_cache<RemoteFileId, TotalPrepSizes, RemoteFileIdHasher> _initialSizesCache;
        std::unordered_set<RemoteFileId, RemoteFileIdHasher> _openedFiles;
        std::string _cacheDir;
    };
}
