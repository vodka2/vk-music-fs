#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/filesystem.hpp>
#include <boost/nowide/fstream.hpp>
#include <json.hpp>
#include "FileCache.h"

namespace bfs = boost::filesystem;
using json = nlohmann::json;
using namespace vk_music_fs;

FileCache::FileCache(
        const std::shared_ptr<net::Mp3SizeObtainer> &sizeObtainer,
        const std::shared_ptr<TagSizeCalculator> &tagSizeCalc,
        const std::shared_ptr<CacheSaver> &cacheSaver,
        SizesCacheSize sizesCacheSize,
        FilesCacheSize filesCacheSize
): _tagSizeCalc(tagSizeCalc), _sizeObtainer(sizeObtainer), _cacheSaver(cacheSaver),
_sizesCache(sizesCacheSize, [](...) -> bool{return true;}),
_initialSizesCache(filesCacheSize, [this](auto file) -> bool{
    if(_openedFiles.find(file) == _openedFiles.end()) {
        bfs::remove(_cacheSaver->constructFilename(file));
        return true;
    } else {
        return false;
    }
}) {
    _cacheSaver->loadSizesFromFile([this] (auto ...args) {_sizesCache.put(args...);});
    _cacheSaver->loadInitialSizesFromFile([this] (auto ...args) {_initialSizesCache.put(args...);});
}

FNameCache FileCache::getFilename(const RemoteFile &file) {
    std::scoped_lock<std::mutex> lock(_initialSizesMutex);
    auto id = RemoteFileId{file};
    _openedFiles.insert(id);
    if(_initialSizesCache.exists(id)){
        if(_initialSizesCache.get(id).totalSize == getFileSize(file)){
            return {_cacheSaver->constructFilename(id), true};
        } else {
            return {_cacheSaver->constructFilename(id), false};
        }
    } else {
        std::string name = _cacheSaver->constructFilename(id);
        _initialSizesCache.put(id, {0, 0});
        return {name, false};
    }
}

uint_fast32_t FileCache::getTagSize(const RemoteFile &file) {
    return _tagSizeCalc->getTagSize(file.getArtist(), file.getTitle());
}

uint_fast32_t FileCache::getFileSize(const RemoteFile &file) {
    std::scoped_lock<std::mutex> lock(_sizesMutex);
    if(_sizesCache.exists(file.getId())){
        return _sizesCache.get(file.getId());
    } else {
        auto size =
                _sizeObtainer->getSize(file.getUri()) +
                _tagSizeCalc->getTagSize(file)
        ;
        _sizesCache.put(
                file.getId(),
                size
        );
        return size;
    }
}

void FileCache::removeSize(const RemoteFileId &fileId) {
    std::scoped_lock<std::mutex> lock(_sizesMutex);
    _sizesCache.remove(fileId);
}

uint_fast32_t FileCache::getUriSize(const RemoteFile &file) {
    return getFileSize(file) - _tagSizeCalc->getTagSize(file);
}

void FileCache::fileClosed(const RemoteFile &file, const TotalPrepSizes &sizes) {
    std::scoped_lock<std::mutex> lock(_initialSizesMutex);
    _openedFiles.erase(file.getId());
    _initialSizesCache.put(file.getId(), sizes);
}

TotalPrepSizes FileCache::getInitialSize(const RemoteFileId &file) {
    std::scoped_lock<std::mutex> lock(_initialSizesMutex);
    return _initialSizesCache.get(file);
}

FileCache::~FileCache() {
    _cacheSaver->saveSizesToFile([this] () {return _sizesCache.getList();});
    _cacheSaver->saveInitialSizesToFile([this] () {return _initialSizesCache.getList();});
}
