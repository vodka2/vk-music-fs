#pragma once

#include <stdexcept>
#include <cstdint>

namespace vk_music_fs {
    class MusicFsException: public std::runtime_error {
    public:
        explicit MusicFsException(const std::string &arg);
    };
}
