#pragma once

#include <net/Mp3SizeObtainer.h>
#include <common/common.h>
#include <mp3core/RemoteFile.h>
#include "CacheSaver.h"
#include "CommonCache.h"

namespace vk_music_fs{
    class FileCache {
    public:
        FileCache(
                const std::shared_ptr<CommonCache<RemoteFile, TotalPrepSizes, Mp3FileSizeHelper>> &commonCache
        );
        void removeSize(const RemoteFileId &fileId);
        FNameCache getFilename(const RemoteFile &file);
        uint_fast32_t getTagSize(const RemoteFile &file);
        uint_fast32_t getFileSize(const RemoteFile &file);
        uint_fast32_t getUriSize(const RemoteFile &file);
        TotalPrepSizes getInitialSize(const RemoteFileId &file);
        void fileClosed(const RemoteFile &file, const TotalPrepSizes &sizes);
    private:
        std::shared_ptr<CommonCache<RemoteFile, TotalPrepSizes, Mp3FileSizeHelper>> _commonCache;
        std::shared_ptr<TagSizeCalculator> _tagSizeCalc;
    };
}
