#include "FsPath.h"
#include "DirOrFile.h"

using namespace vk_music_fs;
using namespace fs;

void FsPath::add(const DirOrFile &part) {
    _parts.push_back(part);
}

DirOrFile FsPath::removeFirst() {
    auto front = _parts.front();
    _parts.pop_front();
    return front;
}

std::list<DirOrFile> &FsPath::getAll() {
    return _parts;
}

const std::list<DirOrFile> &FsPath::cgetAll() const{
    return _parts;
}

void FsPath::unlockAll() {
    for (auto &&part : _parts) {
        part.unlock();
    }
}

DirOrFile FsPath::getLast() {
    return _parts.back();
}

FsPath::FsPath(const std::vector<std::string> &stringParts, int_fast16_t pathSize) : _stringParts(stringParts), _pathSize(pathSize){
}

std::vector<std::string> &FsPath::getStringParts() {
    return _stringParts;
}

bool FsPath::isPathMatched() {
    return _parts.size() == _pathSize;
}

bool FsPath::isParentPathMatched() {
    return _parts.size() == _pathSize - 1;
}

bool FsPath::isParentPathDir() {
    return _parts.front().isDir();
}

bool FsPath::isPathDir() {
    return _parts.back().isDir();
}

FsPathUnlocker::~FsPathUnlocker() {
    for(auto &el: _els){
        el.unlock();
    }
}

FsPathUnlocker::FsPathUnlocker(FsPath &path) {
    for(const auto &el : path.cgetAll()){
        _els.push_back(el);
    }
}

FsPathUnlocker::FsPathUnlocker(const std::vector<FsPath> &paths) {
    for(const auto &path : paths){
        for(const auto &pathEl : path.cgetAll()){
            bool found = false;
            for(const auto &el: _els){
                if(el.getId() == pathEl.getId()){
                    found = true;
                    break;
                }
            }
            if(!found){
                _els.push_back(pathEl);
            }
        }
    }
}
