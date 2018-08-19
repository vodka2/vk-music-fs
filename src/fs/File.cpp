#include "Dir.h"
#include "File.h"

using namespace vk_music_fs;
using namespace vk_music_fs::fs;

File::File(
        std::string name, File::Type type, std::variant<RemoteFile> contents,
        uint_fast32_t time, const DirWPtr &parent
)
        : _name(std::move(name)), _type(type), _contents(std::move(contents)),
        _time(time), _parent(parent) {
}

const std::string File::getName() const {
    return _name;
}

File::Type File::getType() const {
    return _type;
}

RemoteFile File::getRemFile() {
    return std::get<RemoteFile>(_contents);
}

DirPtr File::getParent() {
    return _parent.lock();
}

uint_fast32_t File::getTime() const {
    return _time;
}
