#pragma once

#include "common_fs.h"

namespace vk_music_fs {
    namespace fs {
        class FileName {
        public:
            FileName(const std::string &artist, const std::string &title, const std::string &extension);
            FileName(const std::string &title);
            std::string getFilename();
            void increaseNumberSuffix();

        private:
            std::string escapeName(std::string str);
            std::string _extension;
            std::string _filename;
            bool _useNumberSuffix;
            uint_fast32_t _numberSuffix;
        };
    }
}
