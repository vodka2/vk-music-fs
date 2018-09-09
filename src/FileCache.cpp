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
    loadSizesFromFile();
    loadInitialSizesFromFile();
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
    return (bfs::path(_cacheDir) / (idToStr(file) + ".mp3")).string();
}

TotalPrepSizes FileCache::getInitialSize(const RemoteFileId &file) {
    std::scoped_lock<std::mutex> lock(_initialSizesMutex);
    return _initialSizesCache.get(file);
}

void FileCache::loadSizesFromFile() {
    auto headCacheFile = bfs::path(_cacheDir) / std::string(SIZES_CACHE_FNAME);
    if(bfs::is_regular_file(headCacheFile.c_str())){
        boost::nowide::ifstream strm(headCacheFile.string().c_str());
        auto data = json::parse(strm);
        for(auto it = data.begin(); it != data.end(); ++it){
            _sizesCache.put(
                    RemoteFileId((*it)[0], (*it)[1]),
                    (*it)[2]
            );
        }
    }
}

void FileCache::loadInitialSizesFromFile() {
    auto filesCacheFile = bfs::path(_cacheDir) / std::string(INITIAL_SIZES_CACHE_FNAME);
    if(bfs::is_regular_file(filesCacheFile.c_str())){
        boost::nowide::ifstream strm(filesCacheFile.string().c_str());
        auto data = json::parse(strm);
        for(auto it = data.begin(); it != data.end(); ++it){
            _initialSizesCache.put(
                    RemoteFileId((*it)[0], (*it)[1]),
                    TotalPrepSizes{(*it)[2], (*it)[3]}
            );
        }
    }
}

void FileCache::saveSizesToFile() {
    auto headCacheFile = bfs::path(_cacheDir) / std::string(SIZES_CACHE_FNAME);
    std::vector<std::tuple<int_fast32_t, uint_fast32_t, uint_fast32_t>> vect;
    for(const auto &item : _sizesCache.getList()){
        vect.push_back(std::make_tuple<>(item.first.getOwnerId(), item.first.getFileId(), item.second));
    }
    json out(std::move(vect));
    boost::nowide::ofstream strm(headCacheFile.string().c_str());
    strm << out;
}

void FileCache::saveInitialSizesToFile() {
    auto filesCacheFile = bfs::path(_cacheDir) / std::string(INITIAL_SIZES_CACHE_FNAME);
    std::vector<std::tuple<int_fast32_t, uint_fast32_t, uint_fast32_t, uint_fast32_t>> vect;
    for(const auto &item : _initialSizesCache.getList()){
        vect.push_back(std::make_tuple<>(
                item.first.getOwnerId(), item.first.getFileId(),
                item.second.totalSize, item.second.prependSize
        ));
    }
    json out(std::move(vect));
    boost::nowide::ofstream strm(filesCacheFile.string().c_str());
    strm << out;
}

FileCache::~FileCache() {
    saveSizesToFile();
    saveInitialSizesToFile();
}

std::string FileCache::idToStr(const RemoteFileId &file) {
    return std::to_string(file.getOwnerId()) + "_" + std::to_string(file.getFileId());
}
