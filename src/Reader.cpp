#include "Reader.h"

using namespace vk_music_fs;

Reader::Reader(const CachedFilename &fname) : _strm(fname.t, std::ios::binary | std::ios::in){
}

void Reader::openBlocking() {
}

ByteVect Reader::read(uint_fast32_t offset, uint_fast32_t size) {
    std::scoped_lock<std::mutex> _lock(_readMutex);
    _strm.seekg(offset);
    ByteVect res(size);
    _strm.read(reinterpret_cast<char *>(&res[0]), size);
    return res;
}

Reader::~Reader() {
    _strm.close();
}
