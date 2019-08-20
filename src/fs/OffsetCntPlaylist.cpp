#include "OffsetCntPlaylist.h"

using namespace vk_music_fs;
using namespace fs;
OffsetCntPlaylist::OffsetCntPlaylist(
        uint_fast32_t offset,
        uint_fast32_t cnt,
        const DirPtr &counterDir,
        const DirPtr &refreshDir,
        const PlaylistData &playlist
) : _offset(offset), _cnt(cnt), _counterDir(counterDir), _refreshDir(refreshDir), _playlist(playlist){
}

uint_fast32_t OffsetCntPlaylist::getOffset() const {
    return _offset;
}

uint_fast32_t OffsetCntPlaylist::getCnt() const {
    return _cnt;
}

void OffsetCntPlaylist::setOffset(uint_fast32_t offset) {
    _offset = offset;
}

void OffsetCntPlaylist::setCnt(uint_fast32_t cnt) {
    _cnt = cnt;
}

DirPtr OffsetCntPlaylist::getCounterDir() const {
    return _counterDir.lock();
}

void OffsetCntPlaylist::setCounterDir(const DirPtr &counterDir) {
    _counterDir = counterDir;
}

DirPtr OffsetCntPlaylist::getRefreshDir() const {
    return _refreshDir.lock();
}

void OffsetCntPlaylist::setRefreshDir(const DirPtr &refreshDir) {
    _refreshDir = refreshDir;
}

PlaylistData OffsetCntPlaylist::getPlaylist() const {
    return _playlist;
}

