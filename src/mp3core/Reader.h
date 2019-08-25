#pragma once

#include <common/common.h>
#include <fstream>
#include <mutex>
#include <boost/nowide/fstream.hpp>

namespace vk_music_fs {
    class Reader {
    public:
        Reader(const CachedFilename &fname, const FileSize &size);
        ByteVect read(uint_fast32_t offset, uint_fast32_t size);
        ~Reader();
    private:
        uint_fast32_t _size;
        boost::nowide::ifstream _strm;
        bool _isOpened;
        std::string _fname;
        std::mutex _readMutex;
    };
}
