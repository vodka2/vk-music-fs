#include "WrongSizeException.h"

vk_music_fs::net::WrongSizeException::WrongSizeException(
        uint_fast32_t expected, uint_fast32_t real, const std::string &fname
) : MusicFsException(
        "Expected " + std::to_string(expected) + " bytes got " + std::to_string(real) +
        " when working with " + fname
    ) {}

vk_music_fs::net::WrongSizeException::WrongSizeException(const std::string &fname)
    : MusicFsException(
        "Got wrong file size when working with " + fname
    ) {}
