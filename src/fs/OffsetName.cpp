#include "OffsetName.h"

using namespace vk_music_fs;
using namespace fs;

OffsetName::OffsetName(uint_fast32_t offset, std::string name): _name(std::move(name)), _offset(offset) {
}

const std::string OffsetName::getName() const {
    return _name;
}

uint_fast32_t OffsetName::getOffset() const {
    return _offset;
}
