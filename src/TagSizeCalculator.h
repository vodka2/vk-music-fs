#pragma once

#include <common.h>
#include <RemoteFile.h>

namespace vk_music_fs {
    class TagSizeCalculator {
    public:
        TagSizeCalculator();
        uint_fast32_t getTagSize(const std::string &artist, const std::string &title);
        uint_fast32_t getTagSize(const RemoteFile &file);
    };
}