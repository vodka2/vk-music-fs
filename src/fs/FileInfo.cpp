#include "FileInfo.h"

using namespace vk_music_fs;
using namespace fs;

FileInfo::FileInfo(std::string artist, std::string title, std::string fileName, uint_fast32_t mtime)
: _artist(artist), _title(title), _fileName(fileName), _mtime(mtime){
}

std::string FileInfo::getArtist() const {
    return _artist;
}

std::string FileInfo::getTitle() const {
    return _title;
}

std::string FileInfo::getFileName() const {
    return _fileName;
}

uint_fast32_t FileInfo::getTime() const {
    return _mtime;
}

std::unordered_map<std::string, std::string> FileInfo::toMap() const {
    std::unordered_map<std::string, std::string> ret{};
    ret["artist"] = _artist;
    ret["title"] = _title;
    ret["fileName"] = _fileName;
    return ret;
}
