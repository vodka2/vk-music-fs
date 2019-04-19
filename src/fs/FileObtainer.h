#pragma once

#include "common_fs.h"
#include "FileName.h"
#include "VkException.h"
#include "Dir.h"
#include "FsSettings.h"
#include <json.hpp>
#include <RemoteFile.h>
#include <unordered_set>

using json = nlohmann::json;

namespace vk_music_fs {
    namespace fs {
        template <typename TQueryMaker>
        class FileObtainer {
        public:
            FileObtainer(
                    const std::shared_ptr<TQueryMaker> &queryMaker,
                    const std::shared_ptr<FsSettings> settings
            ): _queryMaker(queryMaker), _settings(settings){
            }

        std::vector<RemoteFile> getMyAudios(uint_fast32_t offset, uint_fast32_t count){
            return getFilenames(parseJson(makeMyAudiosQuery(offset, count)));
        }

        std::vector<RemoteFile> searchBySongName(
                const std::string &searchName,
                uint_fast32_t offset, uint_fast32_t count
        ){
            return getFilenames(parseJson(makeSearchQuery(searchName, offset, count)));
        }

        private:
            std::string makeMyAudiosQuery(
                    uint_fast32_t offset, uint_fast32_t count
            ){
                std::string respStr;
                try{
                    respStr = _queryMaker->makeMyAudiosQuery(offset, count);
                    return std::move(respStr);
                } catch (const json::parse_error &err){
                    throw VkException("Error parsing JSON '" + respStr + "' when obtaining my audios");
                }
            }

            std::string makeSearchQuery(
                    const std::string &searchName,
                    uint_fast32_t offset, uint_fast32_t count
            ){
                std::string respStr;
                try{
                    respStr = _queryMaker->makeSearchQuery(searchName, offset, count);
                    return std::move(respStr);
                } catch (const json::parse_error &err){
                    throw VkException("Error parsing JSON '" + respStr + "' when searching for " + searchName);
                }
            }

            json parseJson(const std::string &str){
                auto res = json::parse(str);
                if(res.find("error") != res.end()) {
                    throw VkException("VK returned error response '" + str + "'");
                }
                return std::move(res);
            }

            std::vector<RemoteFile> getFilenames(
                    json returnedJson
            ) {
                std::vector<RemoteFile> ret;
                std::unordered_set<std::string> _fileNames;
                auto resp = returnedJson["response"];
                for (const auto &item: resp["items"]) {
                    if(!static_cast<std::string>(item["url"]).empty()) {
                        ret.push_back(RemoteFile{item["url"], item["owner_id"],
                                   item["id"], item["artist"], item["title"]});

                    }
                }
                return std::move(ret);
            }

            std::shared_ptr<TQueryMaker> _queryMaker;
            std::shared_ptr<FsSettings> _settings;
        };
    }
}