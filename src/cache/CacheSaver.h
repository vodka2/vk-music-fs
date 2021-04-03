#pragma once

#include <common/common.h>
#include <boost/filesystem.hpp>
#include <boost/nowide/fstream.hpp>
#include <json.hpp>
#include <mp3core/RemoteFile.h>

namespace vk_music_fs{
    struct Meta {
        uint_fast32_t version;
    };

    class CacheSaver {
    public:
        explicit CacheSaver(
                CacheDir cacheDir
        );
        void clearCache();
        std::string getSizesCacheFName();
        std::string getFilesCacheFName();
        std::string getCacheDir();
        std::string constructFilename(const RemoteFileId &file);
        template <typename TContainer>
        void loadSizesFromFile(const TContainer &func) {
            auto headCacheFile = getSizesCacheFName();
            if(boost::filesystem::is_regular_file(headCacheFile.c_str())){
                boost::nowide::ifstream strm(headCacheFile);
                auto data = nlohmann::json::parse(strm);
                for (auto &it : data) {
                    func(
                            RemoteFileId(it[0], it[1]),
                            it[2]
                    );
                }
            }
        }

        template <typename TContainer>
        void loadInitialSizesFromFile(const TContainer &func) {
            auto filesCacheFile = getFilesCacheFName();
            if(boost::filesystem::is_regular_file(filesCacheFile.c_str())){
                boost::nowide::ifstream strm(filesCacheFile);
                auto data = nlohmann::json::parse(strm);
                for (auto &it : data) {
                    func(
                            RemoteFileId(it[0], it[1]),
                            TotalPrepSizes{it[2], it[3]}
                    );
                }
            }
        }

        template <typename TContainer>
        void saveSizesToFile(const TContainer &func) {
            auto headCacheFile = getSizesCacheFName();
            std::vector<std::tuple<int_fast32_t, uint_fast32_t, uint_fast32_t>> vect;
            for(const auto &item : func()){
                vect.push_back(std::make_tuple<>(item.first.getOwnerId(), item.first.getFileId(), item.second));
            }
            nlohmann::json out(std::move(vect));
            boost::nowide::ofstream strm(headCacheFile);
            strm << out;
        }

        template <typename TContainer>
        void saveInitialSizesToFile(const TContainer &func) {
            auto filesCacheFile = getFilesCacheFName();
            std::vector<std::tuple<int_fast32_t, uint_fast32_t, uint_fast32_t, uint_fast32_t>> vect;
            for(const auto &item : func()){
                vect.push_back(std::make_tuple<>(
                        item.first.getOwnerId(), item.first.getFileId(),
                        item.second.totalSize, item.second.prependSize
                ));
            }
            nlohmann::json out(std::move(vect));
            boost::nowide::ofstream strm(filesCacheFile);
            strm << out;
        }
    private:
        Meta getMeta();
        void saveMeta();
        std::string idToStr(const RemoteFileId &file);
        constexpr static std::string_view SIZES_CACHE_FNAME = "head_cache.json";
        constexpr static std::string_view FILES_CACHE_FNAME = "files_cache.json";
        constexpr static std::string_view META_CACHE_FNAME = "meta.json";
        const uint_fast32_t CACHE_VERSION = 1;
        std::string _cacheDir;
        Meta _meta;
    };
}
