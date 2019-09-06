#include "FileBuffer.h"

using namespace vk_music_fs;
namespace bfs = boost::filesystem;

void FileBuffer::open() {
    _path = _cacheDir / bfs::unique_path();
    boost::nowide::ofstream{_path.string()};
    _fs.open(_path.string(), std::ios::binary | std::ios::in | std::ios::out);
    _isClosed = false;
}

FileBuffer::FileBuffer(CacheDir cacheDir) : _file(_fs), _cacheDir(cacheDir.t), _isClosed(true) {
}

FileBuffer::~FileBuffer() {
    close();
}

void FileBuffer::close() {
    if(!_isClosed) {
        _isClosed = true;
        _fs.close();
        bfs::remove(_path);
    }
}
