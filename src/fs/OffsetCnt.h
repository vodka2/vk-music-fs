#pragma once

#include "common_fs.h"

namespace vk_music_fs {
    namespace fs {
        class OffsetCnt {
        public:
            OffsetCnt(uint_fast32_t offset, uint_fast32_t cnt);

            uint_fast32_t getOffset() const;

            uint_fast32_t getCnt() const;

            void setOffset(uint_fast32_t offset);

            void setCnt(uint_fast32_t cnt);
        private:
            uint_fast32_t _offset;
            uint_fast32_t _cnt;
        };
    }
}
