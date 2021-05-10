#pragma once

#include "fs/common_fs.h"
#include <atomic>

namespace vk_music_fs {
    class IdGeneratorBase {
    public:
        IdGeneratorBase();

        uint_fast32_t getNextId();

    protected:
        std::atomic_uint_fast32_t _lastId;
    };

    class IdGenerator: public IdGeneratorBase {
    public: IdGenerator(): IdGeneratorBase() {}
    };
    class ManagerIdGenerator: public IdGeneratorBase {
    public: ManagerIdGenerator(): IdGeneratorBase() {
        _lastId = 100;
    }
    };
}