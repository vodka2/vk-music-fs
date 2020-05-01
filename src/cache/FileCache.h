#pragma once

#include <net/Mp3SizeObtainer.h>
#include <common/common.h>
#include <mp3core/RemoteFile.h>
#include <lrucache.hpp>
#include <mutex>
#include <unordered_set>
#include <mp3core/TagSizeCalculator.h>
#include "CacheSaver.h"

namespace vk_music_fs{
    class FileCache {
    public:
        FileCache(
                const std::shared_ptr<net::Mp3SizeObtainer> &sizeObtainer,
                const std::shared_ptr<TagSizeCalculator> &tagSizeCalc,
                const std::shared_ptr<CacheSaver> &cacheSaver,
                SizesCacheSize sizesCacheSize,
                FilesCacheSize filesCacheSize
        );
        ~FileCache();
        void removeSize(const RemoteFileId &fileId);
        FNameCache getFilename(const RemoteFile &file);
        uint_fast32_t getTagSize(const RemoteFile &file);
        uint_fast32_t getFileSize(const RemoteFile &file);
        uint_fast32_t getUriSize(const RemoteFile &file);
        TotalPrepSizes getInitialSize(const RemoteFileId &file);
        void fileClosed(const RemoteFile &file, const TotalPrepSizes &sizes);
    private:
        static constexpr int_fast32_t OBTAINING_FILE_SIZE = -1;
        std::mutex _initialSizesMutex;
        std::mutex _sizesMutex;
        std::mutex _sizesCondVarMutex;
        std::condition_variable _sizesCondVar;
        std::shared_ptr<CacheSaver> _cacheSaver;
        std::shared_ptr<TagSizeCalculator> _tagSizeCalc;
        std::shared_ptr<net::Mp3SizeObtainer> _sizeObtainer;
        cache::lru_cache<RemoteFileId, int_fast32_t, RemoteFileIdHasher> _sizesCache;
        cache::lru_cache<RemoteFileId, TotalPrepSizes, RemoteFileIdHasher> _initialSizesCache;
        std::unordered_set<RemoteFileId, RemoteFileIdHasher> _openedFiles;
    };
}
