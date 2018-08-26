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

    struct FileOrDirMeta{
        enum class Type{
            FILE_ENTRY,
            DIR_ENTRY,
            NOT_EXISTS
        } type;
        uint_fast32_t time;
    };

    struct FNameCache{
        std::string data;
        bool inCache;
    };
    BOOST_STRONG_TYPEDEF(std::string, Artist); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, Title); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, TagSize); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, FileSize); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, NumSearchFiles); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, Mp3Uri); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, UserAgent); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, Token); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, Mp3Extension); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, CacheDir); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, CachedFilename); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, Mp3CacheSize); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, SizesCacheSize); //NOLINT
    BOOST_STRONG_TYPEDEF(bool, CreateDummyDirs); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, VkUserId); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, FilesCacheSize); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, NumSizeRetries); //NOLINT
    template <typename T>
    class InjPtr: public std::shared_ptr<T>{
    public:
        InjPtr(std::shared_ptr<T> t): std::shared_ptr<T>(t){} //NOLINT
    };

    template <typename ...Ts>
    auto makeInjPtr(Ts... data){
        typedef decltype(boost::di::make_injector(std::forward<Ts>(data)...)) InjType;
        return InjPtr<InjType>{std::make_shared<InjType>(boost::di::make_injector(std::forward<Ts>(data)...))};
    }

    #define auto_init(variable, value) std::decay<decltype(value)>::type variable = value
}