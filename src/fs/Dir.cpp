#include "File.h"
#include "Dir.h"

using namespace vk_music_fs;
using namespace vk_music_fs::fs;

Dir::Dir(
        std::string name, Dir::Type type,
        std::optional<std::variant<OffsetCntName, OffsetCnt>> extra,
        const DirWPtr &parent
) :
        _name(std::move(name)), _type(type), _maxFileNum(0), _extra(std::move(extra)), _parent(parent) {

}

const std::string Dir::getName() const {
    return _name;
}

const DirOrFile Dir::getItem(const std::string &item) const {
    return _contents.find(item)->second;
}

void Dir::addItem(DirOrFile item) {
    if(item.isFile()){
        _maxFileNum++;
    }
    if(item.isDir() && item.dir()->getType() == Dir::Type::COUNTER_DIR){
        _counterDir = item.dir();
    }
    _contents.insert(std::make_pair<>(item.getName(), std::move(item)));
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

OffsetCntName& Dir::getOffsetCntName(){
    return std::get<OffsetCntName>(*_extra);
}

OffsetCnt& Dir::getOffsetCnt(){
    return std::get<OffsetCnt>(*_extra);
}

uint_fast32_t Dir::getMaxFileNum() const {
    return _maxFileNum;
}

DirPtr Dir::getCounterDir() const{
    return *_counterDir;
}
bool Dir::haveCounterDir() const {
    return static_cast<bool>(_counterDir);
}

void Dir::clearContents() {
    _contents.clear();
    _counterDir = std::nullopt;
}

void Dir::clearContentsExceptNested() {
    ContentsMap newMap;
    for(const auto &item : _contents){
        if(item.second.isDir() && item.second.dir()->getType() == Type::SEARCH_DIR){
            newMap.insert(item);
        }
    }
    _contents.clear();
    _counterDir = std::nullopt;
    _contents = std::move(newMap);
}

void Dir::removeCounter() {
    if(haveCounterDir()) {
        removeItem(getCounterDir()->getName());
    }
}
