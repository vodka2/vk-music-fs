#include "RemotePhotoFile.h"

using namespace vk_music_fs;

using namespace vk_music_fs;

RemotePhotoFile::RemotePhotoFile(const std::string &uri, int_fast32_t ownerId, uint_fast32_t albumId):
        _uri(uri), _ownerId(ownerId), _albumId(albumId){
}

const std::string &RemotePhotoFile::getUri() const{
    return _uri;
}

int_fast32_t RemotePhotoFile::getOwnerId() const{
    return _ownerId;
}

uint_fast32_t RemotePhotoFile::getAlbumId() const{
    return _albumId;
}

RemotePhotoFileId RemotePhotoFile::getId() const{
    return RemotePhotoFileId(*this);
}

RemotePhotoFileId::RemotePhotoFileId(const RemotePhotoFile &file): _ownerId(file.getOwnerId()), _albumId(file.getAlbumId()) {
}

int_fast32_t RemotePhotoFileId::getOwnerId() const {
    return _ownerId;
}

uint_fast32_t RemotePhotoFileId::getAlbumId() const {
    return _albumId;
}