#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/filesystem.hpp>
#include <boost/nowide/fstream.hpp>
#include <json.hpp>
#include "FileCache.h"

using namespace vk_music_fs;

FileCache::FileCache(
        const std::shared_ptr<CommonCache<RemoteFile, TotalPrepSizes, Mp3FileSizeHelper>> &commonCache
): _commonCache(commonCache) {}

FNameCache FileCache::getFilename(const RemoteFile &file) {
    return _commonCache->getFilename(file);
}

uint_fast32_t FileCache::getTagSize(const RemoteFile &file) {
    return _commonCache->getExtraSize(file);
}

uint_fast32_t FileCache::getFileSize(const RemoteFile &file) {
    return _commonCache->getFileSize(file);
}

void FileCache::removeSize(const RemoteFileId &fileId) {
    _commonCache->removeSize(fileId);
}

uint_fast32_t FileCache::getUriSize(const RemoteFile &file) {
    return _commonCache->getUriSize(file);
}

void FileCache::fileClosed(const RemoteFile &file, const TotalPrepSizes &sizes) {
    _commonCache->fileClosed(file, sizes);
}

TotalPrepSizes FileCache::getInitialSize(const RemoteFileId &file) {
    return _commonCache->getInitialSize(file);
}
