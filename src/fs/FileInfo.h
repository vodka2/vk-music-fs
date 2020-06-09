#pragma once

#include <string>
#include <unordered_map>

namespace vk_music_fs {
    namespace fs {
        class FileInfo {
        public:
            FileInfo(std::string artist, std::string title, std::string fileName, uint_fast32_t mtime);

            std::string getArtist() const;

            std::string getTitle() const;

            std::string getFileName() const;

            uint_fast32_t getTime() const;

            std::unordered_map<std::string, std::string> toMap() const;

        private:
            uint_fast32_t _mtime;
            std::string _artist;
            std::string _title;
            std::string _fileName;
        };
    }
}
