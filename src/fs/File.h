#pragma once

#include "common_fs.h"
#include <RemoteFile.h>

namespace vk_music_fs {
    namespace fs {
        class File {
        public:
            enum class Type {
                MUSIC_FILE
            };
            File(std::string name, Type type, std::variant<RemoteFile> contents, const DirWPtr &parent);

            const std::string getName() const;

            Type getType() const;

            RemoteFile getRemFile();

            DirPtr getParent();

        private:
            std::string _name;
            Type _type;
            std::variant<RemoteFile> _contents;
            DirWPtr _parent;
        };
    }
}