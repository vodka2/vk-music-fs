#pragma once

#include "File.h"
#include "PhotoCache.h"
#include <string>
#include <fstream>
#include <common/common.h>
#include <mutex>
#include <atomic>
#include <mp3core/RemoteFile.h>
#include <boost/nowide/fstream.hpp>

namespace vk_music_fs {
    class PhotoFile {
    public:
        explicit PhotoFile(const CachedFilename &name, const RemotePhotoFile &remFile, const std::shared_ptr<PhotoCache> &cache);
        
        void open();

        uint_fast32_t getUriSize();

        template <typename TBlock>
        void write(TBlock block) {
            std::scoped_lock <std::mutex> lock(_mutex);
            if(_opened && !_closed) {
                _file.write(block);
            }
        }

        template <typename TBlock>
        void read(uint_fast32_t offset, const TBlock &block) {
            std::scoped_lock <std::mutex> lock(_mutex);
            if(_opened && !_closed) {
                _file.read(offset, block);
            }
        }

        ByteVect read(uint_fast32_t offset, uint_fast32_t size);

        void close();

        uint_fast32_t getSizeOnDisk();

    private:
        bool _closed;
        bool _opened;
        std::string _name;
        RemotePhotoFile _remFile;
        std::shared_ptr<PhotoCache> _cache;
        std::mutex _mutex;
        File _file;
        boost::nowide::fstream _fs;
        std::atomic_uint_fast32_t _totalSize;
    };
}
