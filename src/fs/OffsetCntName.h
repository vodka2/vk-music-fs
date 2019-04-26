#pragma once

#include "common_fs.h"

namespace vk_music_fs {
    namespace fs {
        class OffsetCntName {
        public:
            OffsetCntName(uint_fast32_t offset, uint_fast32_t cnt, std::string name, DirPtr counterDir);

            const std::string getName() const;

            void setName(const std::string &name);

            uint_fast32_t getOffset() const;

            void setOffset(uint_fast32_t offset);

            uint_fast32_t getCnt() const;

            void setCnt(uint_fast32_t cnt);

            DirPtr getCounterDir() const;

            void setCounterDir(const DirPtr &counterDir);

        private:
            std::string _name;
            uint_fast32_t _offset;
            uint_fast32_t _cnt;
            DirWPtr _counterDir;
        };
    }
}