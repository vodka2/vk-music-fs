#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/filesystem.hpp>
#include "FileCache.h"

namespace bfs = boost::filesystem;
using namespace vk_music_fs;

FileCache::FileCache(
        const std::shared_ptr<SizeObtainer> &sizeObtainer,
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
    _openedFiles.insert(file);
    if(_initialSizesCache.exists(file)){
        if(_initialSizesCache.get(file).totalSize == getFileSize(file)){
            return {constructFilename(file), true};
        } else {
            return {constructFilename(file), false};
        }
    } else {
        std::string name = constructFilename(file);
        _initialSizesCache.put(file, {0, 0});
        return {name, false};
    }
}

uint_fast32_t FileCache::getTagSize(const RemoteFile &file) {
    return _sizeObtainer->getTagSize(file.getArtist(), file.getTitle());
}

uint_fast32_t FileCache::getFileSize(const RemoteFile &file) {
    std::scoped_lock<std::mutex> lock(_sizesMutex);
    if(_sizesCache.exists(file)){
        return _sizesCache.get(file);
    } else {
        auto sizeStruct = _sizeObtainer->getSize(file.getUri(), file.getArtist(), file.getTitle());
        uint_fast32_t size = sizeStruct.uriSize + sizeStruct.tagSize;
        _sizesCache.put(file, size);
        return size;
    }
}

void FileCache::fileClosed(const RemoteFile &file, const TotalPrepSizes &sizes) {
    std::scoped_lock<std::mutex> lock(_initialSizesMutex);
    _openedFiles.erase(file);
    _initialSizesCache.put(file, sizes);
}

std::string FileCache::constructFilename(const RemoteFile &file) {
    return (bfs::path(_cacheDir) / (std::to_string(file.getOwnerId()) + "_" + std::to_string(file.getFileId()) + ".mp3")).string();
}

TotalPrepSizes FileCache::getInitialSize(const RemoteFile &file) {
    std::scoped_lock<std::mutex> lock(_initialSizesMutex);
    return _initialSizesCache.get(file);
}
