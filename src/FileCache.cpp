#include "FileCache.h"

using namespace vk_music_fs;

FileCache::FileCache(
        const std::shared_ptr<SizeObtainer> &sizeObtainer,
        SizesCacheSize sizesCacheSize,
        FilesCacheSize filesCacheSize
): _sizeObtainer(sizeObtainer), _sizesCache(sizesCacheSize), _filesCache(filesCacheSize) {

}

std::string FileCache::getFilename(const RemoteFile &file) {
    if(_filesCache.exists(file)){
        return _filesCache.get(file).name;
    } else {
        std::string name = std::to_string(file.getOwnerId()) + "_" + std::to_string(file.getFileId()) + ".mp3";
        _filesCache.put(file, FileCacheItem{name});
        return name;
    }
}

uint_fast32_t FileCache::getTagSize(const RemoteFile &file) {
    return _sizeObtainer->getTagSize(file.getArtist(), file.getTitle());
}

uint_fast32_t FileCache::getFileSize(const RemoteFile &file) {
    if(_sizesCache.exists(file)){
        return _sizesCache.get(file);
    } else {
        auto sizeStruct = _sizeObtainer->getSize(file.getUri(), file.getArtist(), file.getTitle());
        uint_fast32_t size = sizeStruct.uriSize + sizeStruct.tagSize;
        _sizesCache.put(file, size);
        return size;
    }
}

void FileCache::fileClosed(const std::string &fname) {
}
