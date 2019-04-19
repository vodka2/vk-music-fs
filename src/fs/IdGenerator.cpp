#include "IdGenerator.h"

using namespace vk_music_fs;
using namespace vk_music_fs::fs;

IdGenerator::IdGenerator() : _lastId(0){
}

uint_fast32_t IdGenerator::getNextId() {
    return _lastId++;
}
