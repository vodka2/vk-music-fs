#pragma once

#include <net/Mp3SizeObtainer.h>
#include <common/common.h>
#include <common/RemotePhotoFile.h>
#include "CacheSaver.h"
#include "CommonCache.h"

namespace vk_music_fs{
    class PhotoCache {
    public:
        PhotoCache(
                const std::shared_ptr<CommonCache<RemotePhotoFile, TotalSize, DummyFileSizeHelper>> &commonCache
        );
        void removeSize(const RemotePhotoFileId &fileId);
        FNameCache getFilename(const RemotePhotoFile &file);
        uint_fast32_t getFileSize(const RemotePhotoFile &file);
        uint_fast32_t getUriSize(const RemotePhotoFile &file);
        TotalSize getInitialSize(const RemotePhotoFileId &file);
        void fileClosed(const RemotePhotoFile &file, const TotalSize &sizes);
    private:
        std::shared_ptr<CommonCache<RemotePhotoFile, TotalSize, DummyFileSizeHelper>> _commonCache;
        std::shared_ptr<TagSizeCalculator> _tagSizeCalc;
    };
}
