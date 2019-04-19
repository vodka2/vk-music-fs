#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <boost/serialization/strong_typedef.hpp>
#include <boost/di.hpp>
#include <memory>

namespace vk_music_fs{
    typedef std::vector<uint8_t> ByteVect;

    struct TotalPrepSizes{
        uint_fast32_t totalSize;
        uint_fast32_t prependSize;
    };

    struct VkCredentials{
        std::string login;
        std::string password;
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
    BOOST_STRONG_TYPEDEF(uint_fast32_t, HttpTimeout); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, NumSearchFiles); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, Mp3Uri); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, UserAgent); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, Token); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, Mp3Extension); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, CacheDir); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, CachedFilename); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, Mp3CacheSize); //NOLINT
    BOOST_STRONG_TYPEDEF(std::string, ErrLogFile); //NOLINT
    BOOST_STRONG_TYPEDEF(uint_fast32_t, SizesCacheSize); //NOLINT
    BOOST_STRONG_TYPEDEF(bool, CreateDummyDirs); //NOLINT
    BOOST_STRONG_TYPEDEF(bool, LogErrorsToFile); //NOLINT
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

    class BoundPolicy : public boost::di::config {
    public:
        static auto policies(...) noexcept {
            using namespace boost::di::policies;
            return boost::di::make_policies(
                    constructible(is_bound<boost::di::_>{})
            );
        }
    };

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