#pragma once

#include "common_fs.h"
#include "FileName.h"
#include "VkException.h"
#include "Dir.h"
#include "FsSettings.h"
#include <json.hpp>
#include <mp3core/RemoteFile.h>
#include "OffsetCntPlaylist.h"
#include <unordered_set>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <atomic>

using json = nlohmann::json;

namespace vk_music_fs {
    namespace fs {
        template <typename TQueryMaker>
        class FileObtainer {
        private:
            const static uint_fast32_t NO_ID = 0;

        public:
            FileObtainer(
                    const std::shared_ptr<TQueryMaker> &queryMaker,
                    const std::shared_ptr<FsSettings> &settings
            ): _queryMaker(queryMaker), _settings(settings), _userId(NO_ID){
            }

        std::vector<RemoteFile> getMyAudios(uint_fast32_t offset, uint_fast32_t count){
            if(count == 0){
                return {};
            }
            return getFilenames(parseJson(_queryMaker->makeMyAudiosQuery(offset, count)));
        }

        std::vector<RemoteFile> searchSimilar(RemoteFileId fileId, uint_fast32_t offset, uint_fast32_t count) {
            if(count == 0){
                return {};
            }
            return getFilenames(parseJson(_queryMaker->searchSimilar(
                    std::to_string(fileId.getOwnerId()) + "_" + std::to_string(fileId.getFileId()), offset, count
            )));
        }

        void addToMyAudios(int_fast32_t ownerId, uint_fast32_t fileId){
            parseJson(_queryMaker->addToMyAudios(ownerId, fileId));
        }

        void deleteFromMyAudios(int_fast32_t ownerId, uint_fast32_t fileId){
            parseJson(_queryMaker->deleteFromMyAudios(ownerId, fileId));
        }

        void deletePlaylist(int_fast32_t ownerId, uint_fast32_t playlistId){
            parseJson(_queryMaker->deletePlaylist(ownerId, playlistId));
        }

        std::vector<RemoteFile> searchBySongName(
                const std::string &searchName,
                uint_fast32_t offset, uint_fast32_t count
        ){
            if(count == 0){
                return {};
            }
            return getFilenames(parseJson(_queryMaker->makeSearchQuery(searchName, offset, count)));
        }

        std::vector<RemoteFile> searchByArtist(
                const std::string &searchName,
                uint_fast32_t offset, uint_fast32_t count
        ){
            if(count == 0){
                return {};
            }
            return getFilenames(parseJson(_queryMaker->makeArtistSearchQuery(searchName, offset, count)));
        }

        std::vector<PlaylistData> getMyPlaylists(uint_fast32_t offset, uint_fast32_t count){
            if(count == 0){
                return {};
            }
            return getPlaylistData(parseJson(_queryMaker->makeMyPlaylistsQuery(getUserId(), offset, count)));
        }

        std::vector<RemoteFile> getPlaylistAudios(
                const std::string &accessKey, int_fast32_t ownerId, uint_fast32_t albumId,
                uint_fast32_t offset, uint_fast32_t count){
            if(count == 0){
                return {};
            }
            return getFilenames(parseJson(_queryMaker->getPlaylistAudios(accessKey, ownerId, albumId, offset, count)));
        }

        uint_fast32_t getUserId(){
            auto cmp = NO_ID;
            if(_userId.compare_exchange_strong(cmp, NO_ID)){
                _userId = getUserId(parseJson(_queryMaker->getUserId()));
            }
            return _userId;
        }

        private:
            json parseJson(const std::string &str){
                try {
                    auto res = json::parse(str);
                    if (res.find("error") != res.end()) {
                        throw VkException("VK returned error response '" + str + "'");
                    }
                    return std::move(res);
                } catch (const json::parse_error &err){
                    throw VkException(
                            "Error parsing JSON '" + str + "'"
                    );
                }
            }

            std::vector<RemoteFile> getFilenames(
                    json returnedJson
            ) {
                std::vector<RemoteFile> ret;
                auto resp = returnedJson["response"];
                for (const auto &item: resp["items"]) {
                    if(!static_cast<std::string>(item["url"]).empty()) {
                        ret.push_back(RemoteFile{item["url"], item["owner_id"],
                                   item["id"], item["artist"], item["title"]});

                    }
                }
                return std::move(ret);
            }

            std::vector<PlaylistData> getPlaylistData(
                    json returnedJson
            ) {
                std::vector<PlaylistData> ret;
                auto resp = returnedJson["response"];
                for (const auto &item: resp["items"]) {
                    auto isAlbum = (item.find("album_type") != item.end() &&
                            (item["album_type"].get<std::string>() == "main_only" ||
                             item["album_type"].get<std::string>() == "main_feat"));
                    std::optional<std::string> RemotePhotoFile;
                    if (isAlbum && item.find("photo") != item.end()) {
                        auto photo = item["photo"];
                        uint_fast32_t maxPhotoKey = 0;
                        for (auto it = photo.begin(); it != photo.end(); ++it) {
                            if (boost::starts_with(it.key(), "photo_")) {
                                auto curKey = boost::lexical_cast<uint_fast32_t>(boost::replace_first_copy(it.key(), "photo_", ""));
                                if (curKey > maxPhotoKey) {
                                    maxPhotoKey = curKey;
                                }
                            }
                        }
                        if (maxPhotoKey != 0) {
                            RemotePhotoFile = photo["photo_" + std::to_string(maxPhotoKey)].get<std::string>();
                        }
                    }
                    ret.push_back(PlaylistData{item["owner_id"], item["id"], item["access_key"], item["title"], isAlbum, RemotePhotoFile});
                }
                return std::move(ret);
            }

            uint_fast32_t getUserId(
                    json returnedJson
            ) {
                auto resp = returnedJson["response"];
                return resp[0]["id"];
            }

            std::shared_ptr<TQueryMaker> _queryMaker;
            std::shared_ptr<FsSettings> _settings;

            std::atomic_uint_fast32_t _userId;
        };
    }
}