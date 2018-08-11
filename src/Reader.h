#pragma once

#include <common.h>
#include <fstream>
#include <mutex>

namespace vk_music_fs {
    class Reader {
    public:
        explicit Reader(const CachedFilename &fname);
        void openBlocking();
        ByteVect read(uint_fast32_t offset, uint_fast32_t size);
        ~Reader();
    private:
        std::ifstream _strm;
        std::mutex _readMutex;
    };
}
