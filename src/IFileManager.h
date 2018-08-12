#pragma once

#include <common.h>

namespace vk_music_fs{
    class IFileManager {
    public:
        virtual int_fast32_t open(const std::string &filename) = 0;
        virtual ByteVect read(uint_fast32_t id, uint_fast32_t offset, uint_fast32_t size) = 0;
        virtual uint_fast32_t getFileSize(const std::string &filename) = 0;
        virtual void close(uint_fast32_t id) = 0;
    };
}
