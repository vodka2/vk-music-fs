#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "MusicFile.h"

using namespace vk_music_fs;

MusicFile::MusicFile(const CachedFilename &name, const RemoteFile &remFile, const std::shared_ptr<FileCache> &cache)
: _closed(false), _name(name.t), _remFile(remFile), _cache(cache), _file(_fs) {
    auto t = _cache->getInitialSize(_remFile.getId());
    _file.setSizeOnDisk(t.totalSize);
    _prepSize = t.prependSize;
    _totalUriSize = _cache->getUriSize(_remFile);
}

ByteVect MusicFile::read(uint_fast32_t offset, uint_fast32_t size) {
    std::scoped_lock <std::mutex> lock(_mutex);
    if(!_closed) {
        return _file.read(offset, size);
    }
    return {};
}

void MusicFile::close() {
    std::scoped_lock <std::mutex> lock(_mutex);
    if(!_closed) {
        _closed = true;
        _cache->fileClosed(_remFile, {_file.getSizeOnDisk(), _prepSize});
        _fs.close();
    }
}

void MusicFile::finish() {
}

uint_fast32_t MusicFile::getSizeOnDisk() {
    return _file.getSizeOnDisk();
}

void MusicFile::open() {
    if(_file.getSizeOnDisk() == 0) {
        boost::nowide::ofstream{_name.c_str()};
    }
    _fs.open(_name.c_str(), std::ios::binary | std::ios::in | std::ios::out);
}

uint_fast32_t MusicFile::getUriSize() {
    return _totalUriSize;
}

uint_fast32_t MusicFile::getPrependSize() {
    return _prepSize;
}

void MusicFile::setPrependSize(uint_fast32_t size) {
    _prepSize = size;
}
