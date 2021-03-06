#include <mpeg/id3v2/id3v2tag.h>
#include "TagSizeCalculator.h"

using namespace vk_music_fs;

TagSizeCalculator::TagSizeCalculator() { //NOLINT
}

uint_fast32_t TagSizeCalculator::getTagSize(const std::string &artist, const std::string &title) {
    TagLib::ID3v2::Tag tag;
    tag.setTitle(title);
    tag.setArtist(artist);
    return tag.render(4).size();
}

uint_fast32_t TagSizeCalculator::getTagSize(const RemoteFile &file) {
    return getTagSize(file.getArtist(), file.getTitle());
}
