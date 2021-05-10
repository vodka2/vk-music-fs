#pragma once

#include <common/common.h>
#include <boost/filesystem.hpp>
#include <boost/nowide/fstream.hpp>
#include <json.hpp>
#include <mp3core/RemoteFile.h>
#include <common/RemotePhotoFile.h>

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
        std::string getMp3SizesCacheFName();
        std::string getMp3FilesCacheFName();
        std::string getPhotoSizesCacheFName();
        std::string getRemotePhotoFilesCacheFName();
        std::string getCacheDir();
        std::string constructFilename(const RemoteFileId &file);
        std::string constructFilename(const RemotePhotoFileId &file);

        template <typename TId, typename TContainer>
        typename std::enable_if<std::is_same<TId, RemoteFileId>::value, void>::type
        loadSizesFromFile(const TContainer &func) {
            auto headCacheFile = getMp3SizesCacheFName();
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

        template <typename TId, typename TContainer>
        typename std::enable_if<std::is_same<TId, RemoteFileId>::value, void>::type
        loadInitialSizesFromFile(const TContainer &func) {
            auto filesCacheFile = getMp3FilesCacheFName();
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

        template <typename TId, typename TContainer>
        typename std::enable_if<std::is_same<TId, RemoteFileId>::value, void>::type
        saveSizesToFile(const TContainer &func) {
            auto headCacheFile = getMp3SizesCacheFName();
            std::vector<std::tuple<int_fast32_t, uint_fast32_t, uint_fast32_t>> vect;
            for(const auto &item : func()){
                vect.push_back(std::make_tuple<>(item.first.getOwnerId(), item.first.getFileId(), item.second));
            }
            nlohmann::json out(std::move(vect));
            boost::nowide::ofstream strm(headCacheFile);
            strm << out;
        }

        template <typename TId, typename TContainer>
        typename std::enable_if<std::is_same<TId, RemoteFileId>::value, void>::type
        saveInitialSizesToFile(const TContainer &func) {
            auto filesCacheFile = getMp3FilesCacheFName();
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

        template <typename TId, typename TContainer>
        typename std::enable_if<std::is_same<TId, RemotePhotoFileId>::value, void>::type
        loadSizesFromFile(const TContainer &func) {
            auto headCacheFile = getPhotoSizesCacheFName();
            if(boost::filesystem::is_regular_file(headCacheFile.c_str())){
                boost::nowide::ifstream strm(headCacheFile);
                auto data = nlohmann::json::parse(strm);
                for (auto &it : data) {
                    func(
                            RemotePhotoFileId(it[0], it[1]),
                            it[2]
                    );
                }
            }
        }

        template <typename TId, typename TContainer>
        typename std::enable_if<std::is_same<TId, RemotePhotoFileId>::value, void>::type
        loadInitialSizesFromFile(const TContainer &func) {
            auto filesCacheFile = getRemotePhotoFilesCacheFName();
            if(boost::filesystem::is_regular_file(filesCacheFile.c_str())){
                boost::nowide::ifstream strm(filesCacheFile);
                auto data = nlohmann::json::parse(strm);
                for (auto &it : data) {
                    func(
                            RemotePhotoFileId(it[0], it[1]),
                            TotalSize{it[2]}
                    );
                }
            }
        }

        template <typename TId, typename TContainer>
        typename std::enable_if<std::is_same<TId, RemotePhotoFileId>::value, void>::type
        saveSizesToFile(const TContainer &func) {
            auto headCacheFile = getPhotoSizesCacheFName();
            std::vector<std::tuple<int_fast32_t, uint_fast32_t, uint_fast32_t>> vect;
            for(const auto &item : func()){
                vect.push_back(std::make_tuple<>(item.first.getOwnerId(), item.first.getAlbumId(), item.second));
            }
            nlohmann::json out(std::move(vect));
            boost::nowide::ofstream strm(headCacheFile);
            strm << out;
        }

        template <typename TId, typename TContainer>
        typename std::enable_if<std::is_same<TId, RemotePhotoFileId>::value, void>::type
        saveInitialSizesToFile(const TContainer &func) {
            auto filesCacheFile = getRemotePhotoFilesCacheFName();
            std::vector<std::tuple<int_fast32_t, uint_fast32_t, uint_fast32_t>> vect;
            for(const auto &item : func()){
                vect.push_back(std::make_tuple<>(
                        item.first.getOwnerId(), item.first.getAlbumId(),
                        item.second.totalSize
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
        std::string idToStr(const RemotePhotoFileId &file);
        constexpr static std::string_view MP3_SIZES_CACHE_FNAME = "head_cache.json";
        constexpr static std::string_view MP3_FILES_CACHE_FNAME = "files_cache.json";
        constexpr static std::string_view PHOTO_SIZES_CACHE_FNAME = "photo_head_cache.json";
        constexpr static std::string_view PHOTO_FILES_CACHE_FNAME = "photo_files_cache.json";
        constexpr static std::string_view META_CACHE_FNAME = "meta.json";
        const uint_fast32_t CACHE_VERSION = 1;
        std::string _cacheDir;
        Meta _meta;
    };
}
