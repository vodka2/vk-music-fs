#pragma once

#include <boost/filesystem.hpp>
#include <mp3core/common_mp3core.h>
#include <boost/nowide/fstream.hpp>

namespace vk_music_fs {
    class File {
    public:
        File(boost::nowide::fstream &fs);
        template <typename TBlock>
        void read(uint_fast32_t offset, const TBlock &fileIOBlock) {
            _fs.seekg(offset);
            if (offset < _totalSize) {
                auto realSize = std::min<uint32_t>(
                        fileIOBlock->maxSize() - fileIOBlock->curSize(),
                        _totalSize - offset
                );
                _fs.read(reinterpret_cast<char *>(fileIOBlock->addrCurSize()), realSize);
                fileIOBlock->curSize() += realSize;
            }
        }

        template <typename TBlock>
        void write(const TBlock &fileIOBlock) {
            _totalSize += fileIOBlock->curSize();
            _fs.seekp(0, std::ios::end);
            _fs.write(reinterpret_cast<char *>(fileIOBlock->addr()), fileIOBlock->curSize());
            _fs.flush();
        }

        ByteVect read(uint_fast32_t offset, uint_fast32_t size);

        uint_fast32_t getSizeOnDisk() const;

        void setSizeOnDisk(uint_fast32_t size);
    private:
        boost::nowide::fstream &_fs;
        std::atomic_uint_fast32_t  _totalSize;
    };
}
