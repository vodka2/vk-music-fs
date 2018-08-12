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
_filesCache(filesCacheSize, [this](auto file){
    std::remove(constructFilename(file).c_str());
}) {

}

FNameCache FileCache::getFilename(const RemoteFile &file) {
    std::scoped_lock<std::mutex> lock(_filesMutex);
    if(_filesCache.exists(file)){
        return {_filesCache.get(file), true};
    } else {
        std::string name = constructFilename(file);
        _filesCache.put(file, name);
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

void FileCache::fileClosed(const RemoteFile &file, bool isFinished) {
    if(!isFinished){
        _filesCache.remove(file);
    }
}

std::string FileCache::constructFilename(const RemoteFile &file) {
    return std::to_string(file.getOwnerId()) + "_" + std::to_string(file.getFileId()) + ".mp3";
}
