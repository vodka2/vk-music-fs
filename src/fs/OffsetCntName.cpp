#include "OffsetCntName.h"

using namespace vk_music_fs;
using namespace fs;

OffsetCntName::OffsetCntName(uint_fast32_t offset, uint_fast32_t cnt, std::string name, DirPtr counterDir)
: _name(std::move(name)), _offset(offset), _cnt(cnt), _counterDir(counterDir) {
}

const std::string OffsetCntName::getName() const {
    return _name;
}

void OffsetCntName::setName(const std::string &name) {
    _name = name;
}

uint_fast32_t OffsetCntName::getOffset() const {
    return _offset;
}

void OffsetCntName::setOffset(uint_fast32_t offset) {
    _offset = offset;
}

uint_fast32_t OffsetCntName::getCnt() const {
    return _cnt;
}

void OffsetCntName::setCnt(uint_fast32_t cnt) {
    _cnt = cnt;
}

DirPtr OffsetCntName::getCounterDir() const {
    return _counterDir;
}

void OffsetCntName::setCounterDir(const DirPtr &counterDir) {
    _counterDir = counterDir;
}
