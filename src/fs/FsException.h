#pragma once

#include <common/MusicFsException.h>

namespace vk_music_fs {
    namespace fs {
        class FsException : public MusicFsException{
        public:
            explicit FsException(const std::string &arg);
        };
    }
}
