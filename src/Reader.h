#pragma once

#include <common.h>
#include <fstream>
#include <mutex>

namespace vk_music_fs {
    class Reader {
    public:
        Reader(const CachedFilename &fname, const FileSize &size);
        void openBlocking();
        ByteVect read(uint_fast32_t offset, uint_fast32_t size);
        ~Reader();
    private:
        uint_fast32_t _size;
        std::ifstream _strm;
        std::mutex _readMutex;
    };
}
