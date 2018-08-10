#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <boost/serialization/strong_typedef.hpp>

namespace vk_music_fs{
    typedef std::vector<uint8_t> ByteVect;
    struct Mp3FileSize{
        uint_fast32_t uriSize;
        uint_fast32_t tagSize;
    };
    BOOST_STRONG_TYPEDEF(std::string, Artist); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, Title); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, TagSize); //NOLINT
}