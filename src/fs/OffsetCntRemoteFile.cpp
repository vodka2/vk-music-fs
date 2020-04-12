#include "OffsetCntRemoteFile.h"

using namespace vk_music_fs;
using namespace vk_music_fs::fs;

uint_fast32_t OffsetCntRemoteFile::getOffset() const {
    return _offset;
}

void OffsetCntRemoteFile::setOffset(uint_fast32_t offset) {
    _offset = offset;
}

uint_fast32_t OffsetCntRemoteFile::getCnt() const {
    return _cnt;
}

void OffsetCntRemoteFile::setCnt(uint_fast32_t cnt) {
    _cnt = cnt;
}

DirPtr OffsetCntRemoteFile::getCounterDir() const {
    return _counterDir.lock();
}

void OffsetCntRemoteFile::setCounterDir(const DirWPtr &counterDir) {
    _counterDir = counterDir;
}

RemoteFileId OffsetCntRemoteFile::getRemoteFileId() const {
    return _remoteFileId;
}

void OffsetCntRemoteFile::setRemoteFileId(const RemoteFileId &remoteFileId) {
    _remoteFileId = remoteFileId;
}

OffsetCntRemoteFile::OffsetCntRemoteFile(uint_fast32_t offset, uint_fast32_t cnt, const DirWPtr &counterDir, const RemoteFileId &remoteFileId)
: _offset(offset), _cnt(cnt), _counterDir(counterDir), _remoteFileId(remoteFileId) {}
