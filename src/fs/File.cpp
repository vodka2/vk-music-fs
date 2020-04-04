#include "Dir.h"
#include "File.h"

using namespace vk_music_fs;
using namespace vk_music_fs::fs;

File::File(
        std::string name,
        uint_fast32_t id,
        uint_fast32_t time,
        FileExtra extra,
        const DirWPtr &parent
)
        : _name(std::move(name)),
        _time(time), _parent(parent), _id(id), _extra(std::move(extra)), _hidden(false) {
}

const std::string File::getName() const {
    return _name;
}

DirPtr File::getParent() {
    return _parent.lock();
}

uint_fast32_t File::getTime() const {
    return _time;
}

void File::lock() {
    _accessMutex.lock();
}

void File::unlock() {
    _accessMutex.unlock();
}

uint_fast32_t File::getId() const {
    return _id;
}

FileExtra& File::getExtra() {
    return _extra;
}

bool File::isHidden() {
    return _hidden;
}

void File::setHidden(bool hidden) {
    _hidden = hidden;
}
