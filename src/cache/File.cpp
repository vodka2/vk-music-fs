#include "File.h"

using namespace vk_music_fs;

uint_fast32_t File::getSizeOnDisk() const {
    return _totalSize;
}

File::File(boost::nowide::fstream &fs): _fs(fs), _totalSize(0) {

}

ByteVect File::read(uint_fast32_t offset, uint_fast32_t size) {
    _fs.seekg(offset);
    ByteVect res(size);
    _fs.read(reinterpret_cast<char *>(&res[0]), size);
    return res;
}

void File::setSizeOnDisk(uint_fast32_t size) {
    _totalSize = size;
}
