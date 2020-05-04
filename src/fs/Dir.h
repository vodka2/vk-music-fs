#pragma once

#include "common_fs.h"
#include "DirOrFile.h"
#include "OffsetCntName.h"
#include "OffsetCnt.h"
#include "OffsetCntPlaylist.h"
#include "OffsetCntRemoteFile.h"

namespace vk_music_fs {
    namespace fs {
        typedef std::unordered_map<std::string, DirOrFile> ContentsMap;
        class Dir : public std::enable_shared_from_this<Dir>{
        public:
            Dir(
                    std::string name,
                    uint_fast32_t id,
                    DirExtra extra,
                    const DirWPtr &parent
            );
            Dir(
                    std::string name,
                    uint_fast32_t id,
                    DirExtra extra,
                    const DirWPtr &parent,
                    uint_fast32_t time
            );
            const std::string getName() const;
            DirPtr getParent() const;

            void addItem(DirOrFile item);
            bool hasItem(const std::string &name) const;
            void removeItem(const std::string &name);
            const DirOrFile getItem(const std::string &str) const;
            ContentsMap &getContents();
            void clear();
            uint_fast32_t getNumFiles() const;
            uint_fast32_t getId() const;
            DirExtra& getDirExtra();
            void lock();
            void unlock();
            uint_fast32_t getTime() const;
            std::string getAbsolutePath() const;
            FilePtr renameFile(const std::string &oldName, const std::string &newName, uint_fast32_t newId);
        private:
            std::string _name;
            uint_fast32_t _id;
            uint_fast32_t _time;
            ContentsMap _contents;
            DirWPtr _parent;
            DirExtra _extra;
            std::mutex _accessMutex;
        };
    }
}
