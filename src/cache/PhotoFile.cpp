#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "PhotoFile.h"

using namespace vk_music_fs;

PhotoFile::PhotoFile(const CachedFilename &name, const RemotePhotoFile &remFile, const std::shared_ptr<PhotoCache> &cache)
: _closed(false), _opened(false), _name(name.t), _remFile(remFile), _cache(cache), _file(_fs) {
    auto t = _cache->getInitialSize(_remFile.getId());
    _file.setSizeOnDisk(t.totalSize);
    _totalSize = _cache->getUriSize(_remFile);
}

ByteVect PhotoFile::read(uint_fast32_t offset, uint_fast32_t size) {
    std::scoped_lock <std::mutex> lock(_mutex);
    if(_opened && !_closed) {
        return _file.read(offset, size);
    }
    return {};
}

void PhotoFile::close() {
    std::scoped_lock <std::mutex> lock(_mutex);
    if(_opened && !_closed) {
        _closed = true;
        _cache->fileClosed(_remFile, {_file.getSizeOnDisk()});
        _fs.close();
    }
}

uint_fast32_t PhotoFile::getSizeOnDisk() {
    return _file.getSizeOnDisk();
}

void PhotoFile::open() {
    std::scoped_lock <std::mutex> lock(_mutex);
    if (_opened || _closed) {
        return;
    }
    if(_file.getSizeOnDisk() == 0) {
        boost::nowide::ofstream{_name.c_str()};
    }
    _fs.open(_name.c_str(), std::ios::binary | std::ios::in | std::ios::out);
    _opened = true;
}

uint_fast32_t PhotoFile::getUriSize() {
    return _totalSize;
}
