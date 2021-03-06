#include "File.h"
#include "Dir.h"

using namespace vk_music_fs;
using namespace vk_music_fs::fs;

Dir::Dir(
        std::string name,
        uint_fast32_t id,
        DirExtra extra,
        const DirWPtr &parent
) : Dir(name, id, extra, parent, 0){
}

const std::string Dir::getName() const {
    return _name;
}

const DirOrFile Dir::getItem(const std::string &item) const {
    return _contents.find(item)->second;
}

void Dir::addItem(DirOrFile item) {
    _contents.insert(std::make_pair<>(item.getName(), std::move(item)));
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

void Dir::lock() {
    _accessMutex.lock();
}

void Dir::unlock() {
    _accessMutex.unlock();
}

void Dir::clear() {
    _contents.clear();
}

uint_fast32_t Dir::getNumFiles() const {
    return _contents.size();
}

uint_fast32_t Dir::getId() const {
    return _id;
}

DirExtra& Dir::getDirExtra() {
    return _extra;
}

uint_fast32_t Dir::getTime() const {
    return _time;
}

Dir::Dir(
    std::string name, uint_fast32_t id, DirExtra extra, const DirWPtr &parent, uint_fast32_t time
) : _name(std::move(name)), _parent(parent), _id(id), _extra(extra), _time(time){

}

std::string Dir::getAbsolutePath() const {
    return (getParent() != nullptr && getParent()->getName() != "/") ?
        getParent()->getAbsolutePath() + "/" + _name : _name;
}

FilePtr Dir::renameFile(const std::string &oldName, const std::string &newName, uint_fast32_t newId) {
    auto prevFile = getItem(oldName).file();
    auto remFile = std::get<RemoteFile>(*prevFile->getExtra());
    removeItem(prevFile->getName());
    auto newFile = std::make_shared<File>(
            newName,
            newId,
            prevFile->getTime(),
            remFile,
            shared_from_this()
    );
    addItem(newFile);
    return newFile;
}
