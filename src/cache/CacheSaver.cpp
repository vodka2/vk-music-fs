#include "CacheSaver.h"

using namespace vk_music_fs;
namespace bfs = boost::filesystem;

CacheSaver::CacheSaver(CacheDir cacheDir): _cacheDir(cacheDir), _meta(getMeta()) {
    if (_meta.version != CACHE_VERSION) {
        clearCache();
        _meta.version = CACHE_VERSION;
        saveMeta();
    }
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

Meta CacheSaver::getMeta() {
    if (bfs::is_regular_file(bfs::path(getCacheDir()) / std::string(META_CACHE_FNAME))) {
        boost::nowide::ifstream strm((bfs::path(getCacheDir()) / std::string(META_CACHE_FNAME)).string());
        auto data = nlohmann::json::parse(strm);
        return Meta{data.at("version").get<uint_fast32_t>()};
    } else {
        return Meta{0};
    }
}
void CacheSaver::saveMeta() {
    nlohmann::json json;
    json["version"] = _meta.version;
    boost::nowide::ofstream strm((bfs::path(getCacheDir()) / std::string(META_CACHE_FNAME)).string());
    strm << json;
}
