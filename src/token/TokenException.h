#pragma once

#include <common/MusicFsException.h>

namespace vk_music_fs {
    namespace token {
        class TokenException: public MusicFsException {
        public:
            explicit TokenException(const std::string &arg);
        };
    }
}
