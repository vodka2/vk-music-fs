#pragma once

#include "FileCache.h"
#include <string>
#include <fstream>
#include <common.h>
#include <mutex>
#include <atomic>
#include "RemoteFile.h"

namespace vk_music_fs {
    class MusicFile {
    public:
        explicit MusicFile(const CachedFilename &name, const RemoteFile &remFile, const std::shared_ptr<FileCache> &cache);
        
        void open();

        uint_fast32_t getPrependSize();

        void setPrependSize(uint_fast32_t size);

        uint_fast32_t getTotalSize();

        void write(ByteVect vect);

        ByteVect read(uint_fast32_t offset, uint_fast32_t size);
        
        void finish();

        void close();

        uint_fast32_t getSize();

    private:
        std::string _name;
        RemoteFile _remFile;
        std::shared_ptr<FileCache> _cache;
        std::mutex _mutex;
        std::fstream _fs;
        std::atomic_uint_fast32_t _totalInitialSize;
        std::atomic_uint_fast32_t _totalUriSize;
        std::atomic_uint_fast32_t _prepSize;
    };
}
