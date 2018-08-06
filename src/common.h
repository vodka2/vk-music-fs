#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <boost/serialization/strong_typedef.hpp>

namespace vk_music_fs{
    typedef std::vector<uint8_t> ByteVect;
    BOOST_STRONG_TYPEDEF(std::string, Artist); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, Title); //NOLINT
}