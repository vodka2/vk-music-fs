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
                    ret.push_back(PlaylistData{item["owner_id"], item["id"], item["access_key"], item["title"]});
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