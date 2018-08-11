#pragma once

#include <common.h>

namespace vk_music_fs {

    class RemoteFile {
    public:
        friend class RemoteFileHasher;
        RemoteFile(
                const std::string &uri, uint_fast32_t ownerId, uint_fast32_t userId,
                const std::string &artist, const std::string &title
        );
        std::string& getUri();
        std::string& getArtist();
        std::string& getTitle();
        bool operator ==(const RemoteFile &other) const{
            return _fileId == other._fileId && _ownerId == other._ownerId;
        }
    private:
        std::string _uri;
        int_fast32_t _ownerId;
        uint_fast32_t _fileId;
        std::string _artist;
        std::string _title;
    };

    struct RemoteFileHasher{
        std::size_t operator()(const RemoteFile &file) const{
            return (std::hash<int_fast32_t>()(file._ownerId) << 1) ^ std::hash<uint_fast32_t>()(file._fileId); //NOLINT
        }
    };
}
