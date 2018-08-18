#include "File.h"
#include "Dir.h"

using namespace vk_music_fs;
using namespace vk_music_fs::fs;

Dir::Dir(
        std::string name, Dir::Type type, ContentsMap contents,
        std::optional<std::variant<OffsetName, uint_fast32_t>> extra,
        const DirWPtr &parent
) :
        _name(std::move(name)), _type(type),
        _contents(std::move(contents)), _extra(std::move(extra)), _parent(parent) {

}

const std::string Dir::getName() const {
    return _name;
}

const DirOrFile Dir::getItem(const std::string &item) const {
    return _contents.find(item)->second;
}

void Dir::addItem(DirOrFile item) {
    _contents.insert(std::make_pair<>(item.getName(), item));
}

Dir::Type Dir::getType() const {
    return _type;
}

DirPtr Dir::getParent() const {
    return _parent.lock();
}

void Dir::removeItem(const std::string &name) {
    _contents.erase(name);
}

ContentsMap &Dir::getContents() {
    return _contents;
}

bool Dir::hasItem(const std::string &name) const {
    return _contents.find(name) != _contents.end();
}

OffsetName Dir::getOffsetName() const {
    return std::get<OffsetName>(*_extra);
}

uint_fast32_t Dir::getNumber() const {
    return std::get<uint_fast32_t>(*_extra);
}
