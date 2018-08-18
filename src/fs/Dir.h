#pragma once

#include "common_fs.h"
#include "DirOrFile.h"
#include "OffsetName.h"

namespace vk_music_fs {
    namespace fs {
        typedef std::unordered_map<std::string, DirOrFile> ContentsMap;
        class Dir {
        public:
            enum class Type {
                SEARCH_DIR,
                DUMMY_DIR,
                ROOT_DIR,
                ROOT_SEARCH_DIR
            };
            Dir(
                    std::string name,
                    Type type,
                    ContentsMap contents,
                    std::optional<std::variant<OffsetName>> extra,
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
            OffsetName getOffsetName() const;
        private:
            std::string _name;
            Type _type;
            ContentsMap _contents;
            std::optional<std::variant<OffsetName>> _extra;
            DirWPtr _parent;
        };
    }
}
