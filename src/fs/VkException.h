#pragma once

#include <common/MusicFsException.h>

namespace vk_music_fs {
    namespace fs {
        class VkException: public MusicFsException {
        public:
            explicit VkException(const std::string &arg);
        };
    }
}
