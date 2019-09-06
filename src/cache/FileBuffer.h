#pragma once
#include <boost/filesystem.hpp>
#include <mp3core/common_mp3core.h>
#include <boost/nowide/fstream.hpp>
#include "File.h"

namespace vk_music_fs {
    class FileBuffer {
    public:
        void open();

        template <typename TBlock>
        void read(uint_fast32_t offset, const TBlock &fileIOBlock) {
            if (_file.getSizeOnDisk() == 0) {
                open();
            }
            _file.read(offset, fileIOBlock);
        }

        template <typename TBlock>
        void write(const TBlock &fileIOBlock) {
            if (_file.getSizeOnDisk() == 0) {
                open();
            }
            _file.write(fileIOBlock);
        }

        void close();

        explicit FileBuffer(CacheDir cacheDir);

        ~FileBuffer();

    private:
        bool _isClosed;
        boost::filesystem::path _path;
        std::string _cacheDir;
        File _file;
        boost::nowide::fstream _fs;
    };
}
