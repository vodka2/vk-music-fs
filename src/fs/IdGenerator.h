#pragma once

#include "common_fs.h"
#include <atomic>

namespace vk_music_fs {
    namespace fs {
        class IdGenerator {
        public:
            IdGenerator();

            uint_fast32_t getNextId();

        private:
            std::atomic_uint_fast32_t _lastId;
        };
    }
}