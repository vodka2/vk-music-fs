#pragma once

#include <stdexcept>

namespace vk_music_fs {
    class MusicFsException: public std::runtime_error {
    public:
        explicit MusicFsException(const std::string &arg);
    };
}
