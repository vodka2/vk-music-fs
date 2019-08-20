#pragma once

#include "common_fs.h"
#include "FileName.h"

namespace vk_music_fs {
    namespace fs {
        class Mp3FileName {
        public:
            Mp3FileName(const std::string &artist, const std::string &title, const std::string &extension);
            std::string getArtist();
            std::string getTitle();
            std::string getFilename();
            void increaseNumberSuffix();
        private:
            std::string _artist;
            std::string _title;
            FileName _fname;
            std::string replaceDoubleBrackets(std::string str);
        };
    }
}
