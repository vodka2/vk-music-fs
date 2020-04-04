#pragma once

#include "common.h"

namespace vk_music_fs {
    class RealFs {
    public:
        void createFile(const std::string &filename);
    };
}
