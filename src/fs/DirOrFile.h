#pragma once

#include "common_fs.h"

namespace vk_music_fs {
    namespace fs {
        class DirOrFile {
        public:
            DirOrFile(const DirPtr &dir); //NOLINT
            DirOrFile(const FilePtr &file); //NOLINT
            bool isDir() const;
            bool isFile() const;
            DirPtr dir() const;
            FilePtr file() const;
            std::string getName() const;
        private:
            std::variant<DirPtr, FilePtr> _data;
        };
    }
}
