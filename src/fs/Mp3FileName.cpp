#include "Mp3FileName.h"
#include <regex>

using namespace vk_music_fs;
using namespace fs;

std::string Mp3FileName::replaceDoubleBrackets(std::string str) {
    std::regex regex(R"((\([^\(\)]+\))\s*\1$)");
    return std::regex_replace(str, regex, "$1");
}

Mp3FileName::Mp3FileName(const std::string &artist, const std::string &title, const std::string &extension) :
    _artist(artist), _title(replaceDoubleBrackets(title)), _fname(artist, _title, extension){

}

std::string Mp3FileName::getArtist() {
    return _artist;
}

std::string Mp3FileName::getTitle() {
    return _title;
}

std::string Mp3FileName::getFilename() {
    return _fname.getFilename();
}

void Mp3FileName::increaseNumberSuffix() {
    _fname.increaseNumberSuffix();
}
