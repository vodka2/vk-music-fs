#pragma once

#include <mp3core/RemoteFile.h>

namespace vk_music_fs {
    namespace fs {
        class FileCacheM0 {
        public:
            template<typename... T>
            FileCacheM0(T &&... args) {} //NOLINT
            MOCK_CONST_METHOD1(getFileSize, uint_fast32_t(const RemoteFile &file));
        };
    }
}

typedef testing::NiceMock<vk_music_fs::fs::FileCacheM0> FileCacheM;