#pragma once

#include "MusicFsException.h"

namespace vk_music_fs {
    namespace net {
        class WrongSizeException : public MusicFsException {
        public:
            WrongSizeException(
                    uint_fast32_t expected, uint_fast32_t real, const std::string &fname
            );

            explicit WrongSizeException(
                    const std::string &fname
            );
        };
    }
}