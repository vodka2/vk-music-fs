#pragma once

#include "common_fs.h"

namespace vk_music_fs {
    namespace fs {
        class OffsetCnt {
        public:
            OffsetCnt(uint_fast32_t offset, uint_fast32_t cnt, const DirPtr &counterDir, const DirPtr &refreshDir);

            uint_fast32_t getOffset() const;

            uint_fast32_t getCnt() const;

            void setOffset(uint_fast32_t offset);

            void setCnt(uint_fast32_t cnt);

            DirPtr getRefreshDir() const;

            void setRefreshDir(const DirPtr &refreshDir);

            DirPtr getCounterDir() const;

            void setCounterDir(const DirPtr &counterDir);

        private:
            uint_fast32_t _offset;
            uint_fast32_t _cnt;
            DirWPtr _counterDir;
            DirWPtr _refreshDir;
        };
    }
}
