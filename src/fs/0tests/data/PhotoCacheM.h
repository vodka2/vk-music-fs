#pragma once

#include <mp3core/RemoteFile.h>

namespace vk_music_fs {
    namespace fs {
        class PhotoCacheM0 {
        public:
            template<typename... T>
            PhotoCacheM0(T &&... args) {} //NOLINT
            MOCK_CONST_METHOD1(getFileSize, uint_fast32_t(const RemotePhotoFile &file));
        };
    }
}

typedef testing::NiceMock<vk_music_fs::fs::PhotoCacheM0> PhotoCacheM;