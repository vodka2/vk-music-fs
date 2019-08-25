#pragma once

#include <common/MusicFsException.h>

namespace vk_music_fs {
    namespace token {
        class ProtobufException : public MusicFsException{
        public:
            explicit ProtobufException(const std::string &arg);
        };
    }
}
