#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <boost/serialization/strong_typedef.hpp>
#include <boost/di.hpp>
#include <memory>

namespace vk_music_fs{
    typedef std::vector<uint8_t> ByteVect;
    struct Mp3FileSize{
        uint_fast32_t uriSize;
        uint_fast32_t tagSize;
    };
    struct FNameCache{
        std::string data;
        bool inCache;
    };
    BOOST_STRONG_TYPEDEF(std::string, Artist); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, Title); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, TagSize); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, Mp3Uri); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, CachedFilename); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, Mp3CacheSize); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, SizesCacheSize); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, FilesCacheSize); //NOLINT
    template <typename T, typename ...Ts>
    class InjPtr: public std::shared_ptr<boost::di::injector<T, Ts...>>{
    public:
        InjPtr(std::shared_ptr<boost::di::injector<T, Ts...>> t): std::shared_ptr<boost::di::injector<T, Ts...>>(t){} //NOLINT
    };
}