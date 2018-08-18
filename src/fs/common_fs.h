#pragma once

#include <common.h>
#include <unordered_map>
#include <variant>

namespace vk_music_fs{
    namespace fs{
        class Dir;
        class File;
        class DirOrFile;
        class OffsetName;

        typedef std::shared_ptr<Dir> DirPtr;
        typedef std::weak_ptr<Dir> DirWPtr;
        typedef std::shared_ptr<File> FilePtr;

    }
}
