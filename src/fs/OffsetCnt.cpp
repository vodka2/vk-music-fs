#include "OffsetCnt.h"

using namespace vk_music_fs;
using namespace fs;
OffsetCnt::OffsetCnt(uint_fast32_t offset, uint_fast32_t cnt) : _offset(offset), _cnt(cnt){
}

uint_fast32_t OffsetCnt::getOffset() const {
    return _offset;
}

uint_fast32_t OffsetCnt::getCnt() const {
    return _cnt;
}

void OffsetCnt::setOffset(uint_fast32_t offset) {
    _offset = offset;
}

void OffsetCnt::setCnt(uint_fast32_t cnt) {
    _cnt = cnt;
}
