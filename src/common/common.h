#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <boost/serialization/strong_typedef.hpp>
#include <optional>
#include <memory>
#include "OverridableSetting.h"

namespace vk_music_fs{
    typedef std::vector<uint8_t> ByteVect;

    struct TotalPrepSizes{
        uint_fast32_t totalSize;
        uint_fast32_t prependSize;
    };

    struct TotalSize {
        uint_fast32_t totalSize;
    };

    struct VkCredentials{
        std::string login;
        std::string password;
    };

    struct FileOrDirMeta{
        enum class Type{
            FILE_ENTRY,
            FILE_ENTRY_NO_SIZE,
            DIR_ENTRY,
            NOT_EXISTS
        } type;
        uint_fast32_t time;
    };

    struct FNameCache{
        std::string data;
        bool inCache;
    };

    struct SongData {
        std::string artist;
        std::string title;
        std::optional<std::string> albumName;
    };
    BOOST_STRONG_TYPEDEF(uint_fast32_t, TagSize); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, FileSize); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, HttpTimeout); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, NumSearchFiles); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, RemoteFileUri); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, UserAgent); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, Token); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, Mp3Extension); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, CacheDir); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, CachedFilename); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, Mp3CacheSize); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, ErrLogFile); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, SizesCacheSize); //NOLINT
    OVERRIDABLE_SETTING(bool, CreateDummyDirs); //NOLINT
    BOOST_STRONG_TYPEDEF(bool, LogErrorsToFile); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, VkUserId); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, FilesCacheSize); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, NumSizeRetries); //NOLINT

    #define auto_init(variable, value) std::decay<decltype(value)>::type variable = value

    template<std::size_t I = 0, typename FuncT, typename... Tp>
    inline typename std::enable_if<I == sizeof...(Tp), void>::type
    for_each(const std::tuple<Tp...> &, FuncT){}

    template<std::size_t I = 0, typename FuncT, typename... Tp>
    inline typename std::enable_if<I < sizeof...(Tp), void>::type
    for_each(const std::tuple<Tp...>& t, FuncT f){
        if(!f(std::get<I>(t))){
            for_each<I + 1, FuncT, Tp...>(t, f);
        }
    }

}