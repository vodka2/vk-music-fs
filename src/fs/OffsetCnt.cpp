#include "OffsetCnt.h"

using namespace vk_music_fs;
using namespace fs;
OffsetCnt::OffsetCnt(
        uint_fast32_t offset,
        uint_fast32_t cnt,
        const DirPtr &counterDir,
        const DirPtr &refreshDir
) : _offset(offset), _cnt(cnt), _counterDir(counterDir), _refreshDir(refreshDir){
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

DirPtr OffsetCnt::getCounterDir() const {
    return _counterDir.lock();
}

void OffsetCnt::setCounterDir(const DirPtr &counterDir) {
    _counterDir = counterDir;
}

DirPtr OffsetCnt::getRefreshDir() const {
    return _refreshDir.lock();
}

void OffsetCnt::setRefreshDir(const DirPtr &refreshDir) {
    _refreshDir = refreshDir;
}
