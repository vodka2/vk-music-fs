#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "MusicFile.h"

using namespace vk_music_fs;

MusicFile::MusicFile(const CachedFilename &name, const RemoteFile &remFile, const std::shared_ptr<FileCache> &cache)
: _closed(false), _name(name.t), _remFile(remFile), _cache(cache) {
    auto t = _cache->getInitialSize(_remFile);
    _totalInitialSize = t.totalSize;
    _prepSize = t.prependSize;
    _totalUriSize = _cache->getFileSize(_remFile);
}

void MusicFile::write(ByteVect vect) {
    std::scoped_lock <std::mutex> lock(_mutex);
    if(!vect.empty() && !_closed) {
        _totalInitialSize += vect.size();
        _fs.seekp(0, std::ios::end);
        _fs.write(reinterpret_cast<const char *>(&vect[0]), vect.size());
        _fs.flush();
    }
}

ByteVect MusicFile::read(uint_fast32_t offset, uint_fast32_t size) {
    std::scoped_lock <std::mutex> lock(_mutex);
    if(!_closed) {
        _fs.seekg(offset);
        ByteVect res(size);
        _fs.read(reinterpret_cast<char *>(&res[0]), size);
        return res;
    }
    return {};
}

void MusicFile::close() {
    std::scoped_lock <std::mutex> lock(_mutex);
    if(!_closed) {
        _closed = true;
        _cache->fileClosed(_remFile, {_totalInitialSize, _prepSize});
        _fs.close();
    }
}

void MusicFile::finish() {
}

uint_fast32_t MusicFile::getSize() {
    return _totalInitialSize;
}

void MusicFile::open() {
    if(_totalInitialSize == 0) {
        boost::nowide::ofstream{_name.c_str()};
    }
    _fs.open(_name.c_str(), std::ios::binary | std::ios::in | std::ios::out);
}

uint_fast32_t MusicFile::getTotalSize() {
    return _totalUriSize;
}

uint_fast32_t MusicFile::getPrependSize() {
    return _prepSize;
}

void MusicFile::setPrependSize(uint_fast32_t size) {
    _prepSize = size;
}
