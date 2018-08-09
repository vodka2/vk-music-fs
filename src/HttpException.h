#pragma once

#include <stdexcept>

namespace vk_music_fs {
    class HttpException : public std::runtime_error{
    public:
        explicit HttpException(std::string str);
    };
}
