#pragma once

#include <common/common.h>

namespace vk_music_fs {

    class RemoteFileId;

    class RemoteFile {
    public:
        typedef RemoteFileId IdType;
        RemoteFile(
                const std::string &uri, int_fast32_t ownerId, uint_fast32_t fileId,
                const std::string &artist, const std::string &title, const std::optional<std::string> albumName = std::nullopt
        );
        const std::string& getUri() const;
        const std::string& getArtist() const;
        const std::string& getTitle() const;
        int_fast32_t getOwnerId() const;
        uint_fast32_t getFileId() const;
        bool operator ==(const RemoteFile &other) const{
            return _fileId == other._fileId && _ownerId == other._ownerId;
        }
        RemoteFileId getId() const;
        const std::optional<std::string>& getAlbumName() const;

        const SongData &getSongData() const;

    private:
        std::string _uri;
        int_fast32_t _ownerId;
        uint_fast32_t _fileId;
        SongData _songData;
    };

    struct RemoteFileIdHasher;

    class RemoteFileId{
    public:
        friend class RemoteFileIdHasher;
        typedef RemoteFileIdHasher Hasher;
        explicit RemoteFileId(const RemoteFile &file);
        RemoteFileId(int_fast32_t ownerId, uint_fast32_t fileId) : _ownerId(ownerId), _fileId(fileId){}

        int_fast32_t getOwnerId() const;
        uint_fast32_t getFileId() const;
        bool operator ==(const RemoteFileId &other) const{
            return _fileId == other._fileId && _ownerId == other._ownerId;
        }

    private:
        int_fast32_t _ownerId;
        uint_fast32_t _fileId;
    };

    struct RemoteFileIdHasher{
        std::size_t operator()(const RemoteFileId &file) const{
            return (std::hash<int_fast32_t>()(file._ownerId) << 1) ^ std::hash<uint_fast32_t>()(file._fileId); //NOLINT
        }
    };
}
