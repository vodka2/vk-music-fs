#pragma once

#include "FileCache.h"
#include "File.h"
#include <string>
#include <fstream>
#include <common/common.h>
#include <mutex>
#include <atomic>
#include <mp3core/RemoteFile.h>
#include <boost/nowide/fstream.hpp>

namespace vk_music_fs {
    class MusicFile {
    public:
        explicit MusicFile(const CachedFilename &name, const RemoteFile &remFile, const std::shared_ptr<FileCache> &cache);
        
        void open();

        uint_fast32_t getPrependSize();

        void setPrependSize(uint_fast32_t size);

        uint_fast32_t getUriSize();

        template <typename TBlock>
        void write(TBlock block) {
            std::scoped_lock <std::mutex> lock(_mutex);
            _file.write(block);
        }

        template <typename TBlock>
        void read(uint_fast32_t offset, const TBlock &block) {
            std::scoped_lock <std::mutex> lock(_mutex);
            if(!_closed) {
                _file.read(offset, block);
            }
        }

        ByteVect read(uint_fast32_t offset, uint_fast32_t size);

        void finish();

        void close();

        uint_fast32_t getSizeOnDisk();

    private:
        bool _closed;
        std::string _name;
        RemoteFile _remFile;
        std::shared_ptr<FileCache> _cache;
        std::mutex _mutex;
        File _file;
        boost::nowide::fstream _fs;
        std::atomic_uint_fast32_t _totalUriSize;
        std::atomic_uint_fast32_t _prepSize;
    };
}
