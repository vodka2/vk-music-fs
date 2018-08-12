#include "Reader.h"

using namespace vk_music_fs;

Reader::Reader(const CachedFilename &fname, const FileSize &size) :
    _size(size.t),
    _strm(fname.t, std::ios::binary | std::ios::in){
}

void Reader::openBlocking() {
}

ByteVect Reader::read(uint_fast32_t offset, uint_fast32_t size) {
    std::scoped_lock<std::mutex> _lock(_readMutex);
    if(offset >= _size){
        return ByteVect{};
    }
    _strm.seekg(offset);
    auto realSize = std::min(size, _size - offset);
    ByteVect res(realSize);
    _strm.read(reinterpret_cast<char *>(&res[0]), realSize);
    return res;
}

Reader::~Reader() {
    _strm.close();
}
