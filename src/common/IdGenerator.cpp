#include "IdGenerator.h"

using namespace vk_music_fs;
using namespace vk_music_fs::fs;

IdGeneratorBase::IdGeneratorBase() : _lastId(0){
}

uint_fast32_t IdGeneratorBase::getNextId() {
    return _lastId++;
}
