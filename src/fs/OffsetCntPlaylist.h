#pragma once

#include "common_fs.h"

namespace vk_music_fs {
    namespace fs {
        struct PlaylistId {
            int_fast32_t ownerId;
            uint_fast32_t albumId;
            bool operator ==(const PlaylistId &other) const{
                return albumId == other.albumId && ownerId == other.ownerId;
            }
        };

        struct PlaylistIdHasher{
            std::size_t operator()(const PlaylistId &playlistId) const{
                return (std::hash<int_fast32_t>()(playlistId.ownerId) << 1) ^ std::hash<uint_fast32_t>()(playlistId.albumId);
            }
        };

        struct PlaylistData{
            PlaylistData(int_fast32_t ownerId, uint_fast32_t albumId, const std::string &accessKey,
                         const std::string &title, bool isAlbum = false, std::optional<std::string> RemotePhotoFile = std::nullopt);
            int_fast32_t ownerId;
            uint_fast32_t albumId;
            std::string accessKey;
            std::string title;
            bool isAlbum;
            std::optional<std::string> remotePhotoFile;
        };

        class OffsetCntPlaylist {
        public:
            OffsetCntPlaylist(
                    uint_fast32_t offset, uint_fast32_t cnt, const DirPtr &counterDir, const DirPtr &refreshDir,
                    const PlaylistData &playlist
            );

            uint_fast32_t getOffset() const;

            uint_fast32_t getCnt() const;

            void setOffset(uint_fast32_t offset);

            void setCnt(uint_fast32_t cnt);

            DirPtr getRefreshDir() const;

            void setRefreshDir(const DirPtr &refreshDir);

            DirPtr getCounterDir() const;

            void setCounterDir(const DirPtr &counterDir);

            PlaylistData getPlaylist() const;

        private:
            uint_fast32_t _offset;
            uint_fast32_t _cnt;
            DirWPtr _counterDir;
            DirWPtr _refreshDir;
            PlaylistData _playlist;
        };
    }
}
