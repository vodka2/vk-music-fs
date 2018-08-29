#pragma once

#include <common.h>

namespace vk_music_fs{
    namespace token{
        struct AuthData{
            uint64_t id;
            uint64_t token;
            ByteVect idBuf;
        };
    }
}
