#include "CacheSaver.h"

using namespace vk_music_fs;
namespace bfs = boost::filesystem;

CacheSaver::CacheSaver(CacheDir cacheDir): _cacheDir(cacheDir) {
}

void CacheSaver::clearCache() {
    if(bfs::is_regular_file(getSizesCacheFName())) {
        bfs::remove(bfs::path(getSizesCacheFName()));
    }

    if(bfs::is_regular_file(getFilesCacheFName())) {
        loadInitialSizesFromFile([this](const auto &id, const auto &tmp) {
            bfs::remove(bfs::path(constructFilename(id)));
        });
        bfs::remove(bfs::path(getFilesCacheFName()));
    }
}

std::string CacheSaver::getSizesCacheFName() {
    return (bfs::path(_cacheDir) / std::string(SIZES_CACHE_FNAME)).string();
}

std::string CacheSaver::getFilesCacheFName() {
    return (bfs::path(_cacheDir) / std::string(FILES_CACHE_FNAME)).string();
}

std::string CacheSaver::getCacheDir() {
    return _cacheDir;
}

std::string CacheSaver::idToStr(const RemoteFileId &file) {
    return std::to_string(file.getOwnerId()) + "_" + std::to_string(file.getFileId());
}

std::string CacheSaver::constructFilename(const RemoteFileId &file) {
    return (bfs::path(getCacheDir()) / (idToStr(file) + ".mp3")).string();
}
