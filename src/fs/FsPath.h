#pragma once

#include "common_fs.h"
#include <list>

namespace vk_music_fs {
    namespace fs {
        class FsPath {
        public:
            const static int WITH_PARENT_DIR = 2;
            const static int PATH_WITHOUT_CURRENT = 1;
            FsPath(std::vector<std::string> stringParts, int_fast16_t pathSize);
            void add(const DirOrFile &part);
            DirOrFile removeFirst();
            std::list<DirOrFile> & getAll();
            const std::list<DirOrFile> &cgetAll() const;
            DirOrFile getLast();
            DirOrFile getParent();
            std::vector<std::string>& getStringParts();
            void unlockAll();
            bool isPathMatched();
            bool isParentPathMatched();
            bool isParentPathDir();
            bool isPathDir();

        private:
            std::list<DirOrFile> _parts;
            std::vector<std::string> _stringParts;
            std::string _path;
            int_fast16_t _pathSize;
        };

        class FsPathUnlocker{
        public:
            explicit FsPathUnlocker(FsPath &path);
            explicit FsPathUnlocker(const std::vector<FsPath> &paths);
            ~FsPathUnlocker();
        private:
            std::vector<DirOrFile> _els;
        };
    }
}
