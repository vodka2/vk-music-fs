#pragma once

#include "common_fs.h"

namespace vk_music_fs {
    namespace fs {
        class OffsetName {
        public:
            OffsetName(uint_fast32_t offset, std::string name);

            const std::string getName() const;

            uint_fast32_t getOffset() const;

        private:
            std::string _name;
            uint_fast32_t _offset;
        };
    }
}