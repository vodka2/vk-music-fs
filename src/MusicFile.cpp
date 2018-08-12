#include "MusicFile.h"

using namespace vk_music_fs;

MusicFile::MusicFile(const CachedFilename &name) : _name(name.t), _size(0) {
    std::ofstream{name};
    _fs.open(name, std::ios::binary | std::ios::in | std::ios::out);
}

void MusicFile::write(ByteVect vect) {
    std::scoped_lock <std::mutex> lock(_mutex);
    if(!vect.empty()) {
        _size += vect.size();
        _fs.seekp(0, std::ios::end);
        _fs.write(reinterpret_cast<const char *>(&vect[0]), vect.size());
        _fs.flush();
    }
}

ByteVect MusicFile::read(uint_fast32_t offset, uint_fast32_t size) {
    std::scoped_lock <std::mutex> lock(_mutex);
    _fs.seekg(offset);
    ByteVect res(size);
    _fs.read(reinterpret_cast<char *>(&res[0]), size);
    return res;
}

void MusicFile::close() {
    std::scoped_lock <std::mutex> lock(_mutex);
    _fs.close();
}

void MusicFile::finish() {
}

uint_fast32_t MusicFile::getSize() {
    return _size;
}
