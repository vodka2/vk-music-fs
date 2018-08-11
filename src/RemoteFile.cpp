#include "RemoteFile.h"

using namespace vk_music_fs;

RemoteFile::RemoteFile(const std::string &uri, uint_fast32_t ownerId, uint_fast32_t fileId,
                       const std::string &artist, const std::string &title):
    _uri(uri), _ownerId(ownerId), _fileId(fileId), _artist(artist), _title(title) {
}

std::string &RemoteFile::getUri() {
    return _uri;
}

std::string &RemoteFile::getArtist() {
    return _artist;
}

std::string &RemoteFile::getTitle() {
    return _title;
}
