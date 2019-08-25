#include "RemoteFile.h"

using namespace vk_music_fs;

RemoteFile::RemoteFile(const std::string &uri, int_fast32_t ownerId, uint_fast32_t fileId,
                       const std::string &artist, const std::string &title):
    _uri(uri), _ownerId(ownerId), _fileId(fileId), _artist(artist), _title(title) {
}

const std::string &RemoteFile::getUri() const{
    return _uri;
}

const std::string &RemoteFile::getArtist() const{
    return _artist;
}

const std::string &RemoteFile::getTitle() const{
    return _title;
}

int_fast32_t RemoteFile::getOwnerId() const{
    return _ownerId;
}

uint_fast32_t RemoteFile::getFileId() const{
    return _fileId;
}

RemoteFileId RemoteFile::getId() const{
    return RemoteFileId(*this);
}

RemoteFileId::RemoteFileId(const RemoteFile &file): _ownerId(file.getOwnerId()), _fileId(file.getFileId()) {
}

int_fast32_t RemoteFileId::getOwnerId() const {
    return _ownerId;
}

uint_fast32_t RemoteFileId::getFileId() const {
    return _fileId;
}
