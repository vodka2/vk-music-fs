#pragma once

#include "common_fs.h"
#include <mutex>
#include <mp3core/RemoteFile.h>

namespace vk_music_fs {
    namespace fs {
        class File {
        public:
            File(
                    std::string name,
                    uint_fast32_t id,
                    uint_fast32_t time,
                    FileExtra extra,
                    const DirWPtr &parent
            );

            const std::string getName() const;

            DirPtr getParent();

            uint_fast32_t getTime() const;

            void lock();

            void unlock();

            uint_fast32_t getId() const;

            FileExtra& getExtra();

        private:
            std::string _name;
            DirWPtr _parent;
            std::mutex _accessMutex;
            uint_fast32_t _time;
            uint_fast32_t _id;
            FileExtra _extra;
        };
    }
}