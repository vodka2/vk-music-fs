#pragma once

#include <common/MusicFsException.h>

namespace vk_music_fs {
    namespace net {
        class HttpException : public MusicFsException {
        public:
            explicit HttpException(const std::string &arg);
        };
    }
}
