#pragma once

#include <stdexcept>

namespace vk_music_fs {
    class RemoteException : public std::runtime_error{
    public:
        explicit RemoteException(std::string str);
    };
}
