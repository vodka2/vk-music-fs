#pragma once

#include <common.h>
#include <unordered_map>
#include <unordered_set>
#include <RemoteFile.h>
#include <json.hpp>
#include <regex>

using json = nlohmann::json;

namespace vk_music_fs {
    template <typename TQueryMaker>
    class VkApi {
    public:
        VkApi(const std::shared_ptr<TQueryMaker> &queryMaker, const NumSearchFiles &numSearchFiles)
        : _queryMaker(queryMaker), _numSearchFiles(numSearchFiles){
        }

        bool renameDummyDir(const std::string &oldPath, const std::string &newPath){
            std::regex pathRegex("^/?(.+?)$");
            std::smatch mtc;
            std::regex_search(oldPath, mtc, pathRegex);
            auto dirName = mtc[1];
            if(_dummyDirs.find(dirName) != _dummyDirs.end()) {
                _dummyDirs.erase(oldPath);
                createDir(newPath);
                return true;
            }
            return false;
        }

        bool createDir(const std::string &dirPath){
            std::regex pathRegex("^/?(.+?)$");
            std::smatch mtc;
            std::regex_search(dirPath, mtc, pathRegex);
            auto dirName = mtc[1];
            auto res = json::parse(_queryMaker->makeSearchQuery(dirName, _numSearchFiles));
            auto resp = res["response"];
            for(const auto &item: resp["items"]){
                _searchMap[dirName].files.insert(
                        std::make_pair<>(
                            genFileName(item["artist"], item["title"]),
                            RemoteFile{item["url"], item["owner_id"], item["id"], item["artist"], item["title"]}
                        )
                );
            }
            return true;
        }

        std::vector<std::string> getEntries(const std::string &dirPath){
            std::vector<std::string> ret;
            if(dirPath == "/") {
                ret.reserve(_searchMap.size() + _dummyDirs.size());
                for (const auto &item : _searchMap) {
                    ret.push_back(item.first);
                }
                for (const auto &item : _dummyDirs){
                    ret.push_back(item);
                }
                return ret;
            }
            std::regex dirRegex("^/?(.+?)$");
            std::smatch mtc;
            if(std::regex_search(dirPath, mtc, dirRegex)) {
                auto dirName = mtc[1];
                ret.reserve(_searchMap[dirName].files.size());
                for (const auto &item : _searchMap[dirName].files) {
                    ret.push_back(item.first);
                }
                return ret;
            }
            return ret;
        }

        FileOrDirType getType(const std::string &path){
            if(path == "/"){
                return FileOrDirType::DIR_ENTRY;
            }
            std::regex dirRegex("^/?(.+?)$");
            std::vector<std::string> ret;
            std::smatch mtc;
            if(std::regex_search(path, mtc, dirRegex) &&
                    (_searchMap.find(mtc[1]) != _searchMap.end() || _dummyDirs.find(mtc[1]) != _dummyDirs.end())) {
                return FileOrDirType::DIR_ENTRY;
            }
            std::regex fileRegex("^/?(.+?)/(.+?)$");
            if(
                    std::regex_search(path, mtc, fileRegex) &&
                    _searchMap.find(mtc[1]) != _searchMap.end() &&
                    _searchMap[mtc[1]].files.find(mtc[2]) != _searchMap[mtc[1]].files.end()
            ){
                return FileOrDirType::FILE_ENTRY;
            }
            return FileOrDirType::NOT_EXISTS;
        }

        RemoteFile getRemoteFile(const std::string &path){
            std::regex pathRegex("^/?(.+?)/(.+?)$");
            std::smatch mtc;
            std::regex_search(path, mtc, pathRegex);
            auto dirName = mtc[1];
            auto fileName = mtc[2];
            return _searchMap[dirName].files.find(fileName)->second;
        }

        bool createDummyDir(const std::string &dirPath){
            std::regex pathRegex("^/?(.+?)$");
            std::smatch mtc;
            std::regex_search(dirPath, mtc, pathRegex);
            auto dirName = mtc[1];
            _dummyDirs.insert(dirName);
            return true;
        }

    private:

        std::string genFileName(const std::string &artist, const std::string &title){
            return artist + " - " + title + ".mp3";
        }

        struct SearchEntry{
            std::unordered_map<std::string, RemoteFile> files;
        };
        std::unordered_map<std::string, SearchEntry> _searchMap;
        std::unordered_set<std::string> _dummyDirs;
        std::shared_ptr<TQueryMaker> _queryMaker;
        uint_fast32_t _numSearchFiles;
    };
}
