#include "Reader.h"

using namespace vk_music_fs;

Reader::Reader(const CachedFilename &fname, const FileSize &size) :
    _size(size.t), _fname(fname.t), _isOpened(false), _isClosed(false){
}

ByteVect Reader::read(uint_fast32_t offset, uint_fast32_t size) {
    std::scoped_lock<std::mutex> _lock(_readMutex);
    if(offset >= _size || _isClosed){
        return ByteVect{};
    }
    if(!_isOpened){
        _isOpened = true;
        _strm.open(_fname.c_str(), std::ios::binary | std::ios::in);
    }
    _strm.seekg(offset);
    auto realSize = std::min(size, _size - offset);
    ByteVect res(realSize);
    _strm.read(reinterpret_cast<char *>(&res[0]), realSize);
    return res;
}

Reader::~Reader() {
    close();
}

void Reader::close() {
    std::scoped_lock<std::mutex> _lock(_readMutex);
    if(_isOpened && !_isClosed) {
        _isClosed = true;
        _strm.close();
    }
}
