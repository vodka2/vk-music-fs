#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "FileCache.h"

using namespace vk_music_fs;

FileCache::FileCache(
        const std::shared_ptr<SizeObtainer> &sizeObtainer,
        SizesCacheSize sizesCacheSize,
        FilesCacheSize filesCacheSize
): _sizeObtainer(sizeObtainer),
_sizesCache(sizesCacheSize, [](...){}),
_initialSizesCache(filesCacheSize, [this](auto file){
    std::remove(constructFilename(file).c_str());
}) {

}

FNameCache FileCache::getFilename(const RemoteFile &file) {
    std::scoped_lock<std::mutex> lock(_initialSizesMutex);
    if(_initialSizesCache.exists(file)){
        if(_initialSizesCache.get(file) == getFileSize(file)){
            return {constructFilename(file), true};
        } else {
            return {constructFilename(file), false};
        }
    } else {
        std::string name = constructFilename(file);
        _initialSizesCache.put(file, 0);
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

void FileCache::fileClosed(const RemoteFile &file, uint_fast32_t curSize) {
    std::scoped_lock<std::mutex> lock(_initialSizesMutex);
    _initialSizesCache.put(file, curSize);
}

std::string FileCache::constructFilename(const RemoteFile &file) {
    return std::to_string(file.getOwnerId()) + "_" + std::to_string(file.getFileId()) + ".mp3";
}

uint_fast32_t FileCache::getInitialSize(const RemoteFile &file) {
    std::scoped_lock<std::mutex> lock(_initialSizesMutex);
    return _initialSizesCache.get(file);
}
