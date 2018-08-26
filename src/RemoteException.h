#pragma once

#include "MusicFsException.h"

namespace vk_music_fs {
    class RemoteException : public MusicFsException{
    public:
        explicit RemoteException(const std::string &arg);
    };
}
