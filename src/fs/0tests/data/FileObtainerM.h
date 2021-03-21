#pragma once

namespace vk_music_fs {
    namespace fs {
        class FileObtainerM0 {
        public:
            template<typename... T>
            FileObtainerM0(T &&... args) {}//NOLINT
            MOCK_CONST_METHOD2(getMyPlaylists,
                    std::vector<vk_music_fs::fs::PlaylistData>(
                        uint_fast32_t offset,
                        uint_fast32_t
                    )
            );
            MOCK_CONST_METHOD5(
                    getPlaylistAudios,
                        std::vector<vk_music_fs::RemoteFile>(
                        const std::string &, int_fast32_t, uint_fast32_t, uint_fast32_t, uint_fast32_t
                    )
            );
            MOCK_CONST_METHOD2(
                    deletePlaylist,
                    std::vector<vk_music_fs::RemoteFile>(int_fast32_t, uint_fast32_t)
            );
            MOCK_CONST_METHOD3(searchSimilar,
                    std::vector<RemoteFile>(RemoteFileId, uint_fast32_t, uint_fast32_t)
            );
        };
    }
}
typedef testing::NiceMock<vk_music_fs::fs::FileObtainerM0> FileObtainerM;
