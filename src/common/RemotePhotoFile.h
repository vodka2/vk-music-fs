#pragma once

#include "common.h"

namespace vk_music_fs {
    class RemotePhotoFileId;

    class RemotePhotoFile {
    public:
        typedef RemotePhotoFileId IdType;
        RemotePhotoFile(const std::string &uri, int_fast32_t ownerId, uint_fast32_t albumId);
        const std::string& getUri() const;
        int_fast32_t getOwnerId() const;
        uint_fast32_t getAlbumId() const;
        bool operator ==(const RemotePhotoFile &other) const{
            return _albumId == other._albumId && _ownerId == other._ownerId;
        }
        RemotePhotoFileId getId() const;

    private:
        std::string _uri;
        int_fast32_t _ownerId;
        uint_fast32_t _albumId;
    };

    struct RemotePhotoFileIdHasher;

    class RemotePhotoFileId{
    public:
        friend class RemotePhotoFileIdHasher;
        typedef RemotePhotoFileIdHasher Hasher;
        explicit RemotePhotoFileId(const RemotePhotoFile &file);
        RemotePhotoFileId(int_fast32_t ownerId, uint_fast32_t albumId) : _ownerId(ownerId), _albumId(albumId){}

        int_fast32_t getOwnerId() const;
        uint_fast32_t getAlbumId() const;
        bool operator ==(const RemotePhotoFileId &other) const{
            return _albumId == other._albumId && _ownerId == other._ownerId;
        }

    private:
        int_fast32_t _ownerId;
        uint_fast32_t _albumId;
    };

    struct RemotePhotoFileIdHasher{
        std::size_t operator()(const RemotePhotoFileId &file) const{
            return (std::hash<int_fast32_t>()(file._ownerId) << 1) ^ std::hash<uint_fast32_t>()(file._albumId); //NOLINT
        }
    };
}