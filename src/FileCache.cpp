#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/filesystem.hpp>
#include "FileCache.h"

namespace bfs = boost::filesystem;
using namespace vk_music_fs;

FileCache::FileCache(
        const std::shared_ptr<net::SizeObtainer> &sizeObtainer,
        SizesCacheSize sizesCacheSize,
        FilesCacheSize filesCacheSize,
        CacheDir cacheDir
): _sizeObtainer(sizeObtainer),
_sizesCache(sizesCacheSize, [](...) -> bool{return true;}),
_initialSizesCache(filesCacheSize, [this](auto file) -> bool{
    if(_openedFiles.find(file) == _openedFiles.end()) {
        bfs::remove(constructFilename(file));
        return true;
    } else {
        return false;
    }
}), _cacheDir(cacheDir.t) {

}

FNameCache FileCache::getFilename(const RemoteFile &file) {
    std::scoped_lock<std::mutex> lock(_initialSizesMutex);
    auto id = RemoteFileId{file};
    _openedFiles.insert(id);
    if(_initialSizesCache.exists(id)){
        if(_initialSizesCache.get(id).totalSize == getFileSize(file)){
            return {constructFilename(id), true};
        } else {
            return {constructFilename(id), false};
        }
    } else {
        std::string name = constructFilename(id);
        _initialSizesCache.put(id, {0, 0});
        return {name, false};
    }
}

uint_fast32_t FileCache::getTagSize(const RemoteFile &file) {
    return _sizeObtainer->getTagSize(file.getArtist(), file.getTitle());
}

uint_fast32_t FileCache::getFileSize(const RemoteFile &file) {
    std::scoped_lock<std::mutex> lock(_sizesMutex);
    if(_sizesCache.exists(file.getId())){
        return _sizesCache.get(file.getId());
    } else {
        auto sizeStruct = _sizeObtainer->getSize(file.getUri(), file.getArtist(), file.getTitle());
        uint_fast32_t size = sizeStruct.uriSize + sizeStruct.tagSize;
        _sizesCache.put(file.getId(), size);
        return size;
    }
}

void FileCache::fileClosed(const RemoteFile &file, const TotalPrepSizes &sizes) {
    std::scoped_lock<std::mutex> lock(_initialSizesMutex);
    _openedFiles.erase(file.getId());
    _initialSizesCache.put(file.getId(), sizes);
}

std::string FileCache::constructFilename(const RemoteFileId &file) {
    return (bfs::path(_cacheDir) / (std::to_string(file.getOwnerId()) + "_" + std::to_string(file.getFileId()) + ".mp3")).string();
}

TotalPrepSizes FileCache::getInitialSize(const RemoteFileId &file) {
    std::scoped_lock<std::mutex> lock(_initialSizesMutex);
    return _initialSizesCache.get(file);
}
