#include <boost/range/algorithm_ext/erase.hpp>
#include <regex>
#include "FileName.h"

using namespace vk_music_fs;
using namespace fs;

FileName::FileName(
        const std::string &artist, const std::string &title, const std::string &extension
) : _artist(artist), _title(title), _extension(extension), _useNumberSuffix(false){
    _title = replaceDoubleBrackets(_title);
    _filename = escapeName(_artist +  " - " + _title);
}

std::string FileName::getArtist() {
    return _artist;
}

std::string FileName::getTitle() {
    return _title;
}

std::string FileName::getFilename() {
    if(_useNumberSuffix) {
        return _filename + "_" + std::to_string(_numberSuffix) + _extension;
    } else {
        return _filename + _extension;
    }
}

void FileName::increaseNumberSuffix() {
    if(_useNumberSuffix){
        _numberSuffix++;
    } else {
        _useNumberSuffix = true;
        _numberSuffix = 2;
    }
}

std::string FileName::escapeName(std::string str) {
    boost::remove_erase_if(str, [](auto ch0) {
        unsigned char ch = ch0;
        return ch <= 31 || std::string("<>:/\\|?*").find(ch) != std::string::npos;
    });
    std::replace(str.begin(), str.end(), '"', '\'');
    return str;
}

std::string FileName::replaceDoubleBrackets(std::string str) {
    std::regex regex(R"((\([^\(\)]+\))\s*\1$)");
    return std::regex_replace(str, regex, "$1");
}
