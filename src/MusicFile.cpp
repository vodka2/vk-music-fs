#include "MusicFile.h"

using namespace vk_music_fs;

MusicFile::MusicFile(const std::string &name) : _name(name), _size(0), _finished(false) {
    std::ofstream{name};
    _fs.open(name, std::ios::binary | std::ios::in | std::ios::out);
}

void MusicFile::write(ByteVect vect) {
    std::scoped_lock <std::mutex> lock(_mutex);
    _size += vect.size();
    _fs.write(reinterpret_cast<const char *>(&vect[0]), vect.size());
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
    if (!_finished) {
        std::remove(_name.c_str());
    }
}

void MusicFile::finish() {
    _finished = true;
}

uint_fast32_t MusicFile::getSize() {
    return _size;
}
