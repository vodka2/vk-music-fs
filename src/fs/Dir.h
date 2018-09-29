#pragma once

#include "common_fs.h"
#include "DirOrFile.h"
#include "OffsetCntName.h"
#include "OffsetCnt.h"

namespace vk_music_fs {
    namespace fs {
        typedef std::unordered_map<std::string, DirOrFile> ContentsMap;
        class Dir {
        public:
            enum class Type {
                ROOT_MY_AUDIOS_DIR,
                COUNTER_DIR,
                SEARCH_DIR,
                DUMMY_DIR,
                ROOT_DIR,
                ROOT_SEARCH_DIR,
                ARTIST_SEARCH_DIR,
                ROOT_ARTIST_SEARCH_DIR
            };
            Dir(
                    std::string name,
                    Type type,
                    std::optional<std::variant<OffsetCntName, OffsetCnt>> extra,
                    const DirWPtr &parent
            );
            const std::string getName() const;

            Type getType() const;
            DirPtr getParent() const;

            void addItem(DirOrFile item);
            bool hasItem(const std::string &name) const;
            void removeItem(const std::string &name);
            const DirOrFile getItem(const std::string &str) const;
            ContentsMap &getContents();
            OffsetCntName& getOffsetCntName();
            OffsetCnt& getOffsetCnt();
            uint_fast32_t getMaxFileNum() const;
            DirPtr getCounterDir() const;
            bool haveCounterDir() const;
            void removeCounter();
            void clearContents();
            void clearContentsExceptNested();
        private:
            std::string _name;
            Type _type;
            ContentsMap _contents;
            std::optional<std::variant<OffsetCntName, OffsetCnt>> _extra;
            std::optional<DirPtr> _counterDir;
            DirWPtr _parent;
            uint_fast32_t _maxFileNum;
        };
    }
}
