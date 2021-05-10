#include "PhotoCache.h"

using namespace vk_music_fs;

PhotoCache::PhotoCache(
        const std::shared_ptr<CommonCache<RemotePhotoFile, TotalSize, DummyFileSizeHelper>> &commonCache
): _commonCache(commonCache) {}

FNameCache PhotoCache::getFilename(const RemotePhotoFile &file) {
    return _commonCache->getFilename(file);
}

uint_fast32_t PhotoCache::getFileSize(const RemotePhotoFile &file) {
    return _commonCache->getFileSize(file);
}

void PhotoCache::removeSize(const RemotePhotoFileId &fileId) {
    _commonCache->removeSize(fileId);
}

uint_fast32_t PhotoCache::getUriSize(const RemotePhotoFile &file) {
    return _commonCache->getUriSize(file);
}

void PhotoCache::fileClosed(const RemotePhotoFile &file, const TotalSize &sizes) {
    _commonCache->fileClosed(file, sizes);
}

TotalSize PhotoCache::getInitialSize(const RemotePhotoFileId &file) {
    return _commonCache->getInitialSize(file);
}
