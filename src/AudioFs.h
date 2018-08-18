#pragma once

#include <iostream>
#include <utility>
#include <common.h>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <RemoteFile.h>
#include <json.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <algorithm>
#include <mutex>

using json = nlohmann::json;

namespace vk_music_fs {
    template <typename TQueryMaker>
    class AudioFs {
    public:
        AudioFs(const std::shared_ptr<TQueryMaker> &queryMaker, const NumSearchFiles &numSearchFiles)
        : _queryMaker(queryMaker), _numSearchFiles(numSearchFiles),
          _rootDir(std::make_shared<Dir>("/", Dir::Type::ROOT_DIR, ContentsMap{},  std::nullopt, DirWPtr{})){
            std::shared_ptr<Dir> searchDir = std::make_shared<Dir>(
                    "Search", Dir::Type::ROOT_SEARCH_DIR, ContentsMap{}, std::nullopt, _rootDir
            );
            _rootDir->contents.insert(std::make_pair<>(std::string("Search"), searchDir));
        }

        bool renameDummyDir(const std::string &oldPath, const std::string &newPath){
            std::scoped_lock<std::mutex> lock(_fsMutex);
            auto oldDirO = findPath(oldPath);
            auto newDirO = findPath(newPath, 1);
            if(
                    isDir(oldDirO) &&
                    (
                    std::get<DirPtr>(*oldDirO)->parent.lock()->type == Dir::Type::ROOT_SEARCH_DIR ||
                    std::get<DirPtr>(*oldDirO)->parent.lock()->type == Dir::Type::SEARCH_DIR
                    )
                    &&
                    std::get<DirPtr>(*oldDirO)->type == Dir::Type::DUMMY_DIR &&
                    isDir(newDirO) &&
                    std::get<DirPtr>(*oldDirO)->parent.lock() == std::get<DirPtr>(*newDirO)
            ){
                auto oldDir = std::get<DirPtr>(*oldDirO);
                oldDir->parent.lock()->contents.erase(oldDir->name);
                return createDirNoLock(newPath);
            }
            return false;
        }

        bool createDir(const std::string &dirPath){
            std::scoped_lock<std::mutex> lock(_fsMutex);
            return createDirNoLock(dirPath);
        }

        std::vector<std::string> getEntries(const std::string &dirPath){
            std::scoped_lock<std::mutex> lock(_fsMutex);
            std::vector<std::string> ret;
            auto dirO = findPath(dirPath);
            if(!isDir(dirO)){
                return ret;
            }
            auto dir = std::get<DirPtr>(*dirO);
            for (const auto &item : dir->contents) {
                ret.push_back(item.first);
            }
            return ret;
        }

        FileOrDirType getType(const std::string &path){
            std::scoped_lock<std::mutex> lock(_fsMutex);
            auto pathO = findPath(path);
            if(pathO){
                if(isDir(pathO)){
                    return FileOrDirType::DIR_ENTRY;
                } else {
                    return FileOrDirType::FILE_ENTRY;
                }
            }
            return FileOrDirType::NOT_EXISTS;
        }

        RemoteFile getRemoteFile(const std::string &path){
            std::scoped_lock<std::mutex> lock(_fsMutex);
            auto pathO = findPath(path);
            return std::get<RemoteFile>(std::get<FilePtr>(*pathO)->contents);
        }

        bool createDummyDir(const std::string &path){
            std::scoped_lock<std::mutex> lock(_fsMutex);
            auto pathO = findPath(path, 1);
            if(isDir(pathO)){
                auto dirName = getLast(path);
                auto parentDir = std::get<DirPtr>(*pathO);
                parentDir->contents.insert(std::make_pair(
                        dirName,
                        std::make_shared<Dir>(dirName, Dir::Type::DUMMY_DIR, ContentsMap{}, std::nullopt, DirWPtr{parentDir})
                ));
                return true;
            }
            return false;
        }

        bool deleteDir(const std::string &path){
            std::scoped_lock<std::mutex> lock(_fsMutex);
            auto pathO = findPath(path);
            if(isDir(pathO)
                &&
                    (
                            std::get<DirPtr>(*pathO)->type == Dir::Type::SEARCH_DIR ||
                            std::get<DirPtr>(*pathO)->type == Dir::Type::DUMMY_DIR
                    )
            ){
                auto dirName = getLast(path);
                auto parentDir = std::get<DirPtr>(*pathO)->parent.lock();
                parentDir->contents.erase(dirName);
                return true;
            }
            return false;
        }

        bool deleteFile(const std::string &path){
            std::scoped_lock<std::mutex> lock(_fsMutex);
            auto pathO = findPath(path);
            if(isFile(pathO) && std::get<FilePtr>(*pathO)->type == File::Type::MUSIC_FILE){
                auto fileName = getLast(path);
                auto parentDir = std::get<FilePtr>(*pathO)->parent.lock();
                parentDir->contents.erase(fileName);
                return true;
            }
            return false;
        }

    private:
        struct Dir;
        struct File;

        typedef std::shared_ptr<Dir> DirPtr;
        typedef std::weak_ptr<Dir> DirWPtr;
        typedef std::shared_ptr<File> FilePtr;
        typedef std::unordered_map<std::string, std::variant<std::shared_ptr<Dir>, std::shared_ptr<File>>> ContentsMap;

        struct File{
            std::string name;
            enum class Type{
                MUSIC_FILE
            } type;
            std::variant<RemoteFile> contents;
            DirWPtr parent;
            File(std::string _name, Type _type, std::variant<RemoteFile> _contents, DirWPtr _parent)
            :name(std::move(_name)), type(_type), contents(std::move(_contents)), parent(_parent){
            }
        };

        struct OffsetName{
            uint_fast32_t offset;
            std::string name;
            OffsetName(uint_fast32_t _offset, std::string _name) : offset(_offset), name(std::move(_name)){
            }
        };

        struct Dir{
            std::string name;
            enum class Type{
                SEARCH_DIR,
                DUMMY_DIR,
                ROOT_DIR,
                ROOT_SEARCH_DIR
            } type;
            ContentsMap contents;
            std::optional<std::variant<OffsetName>> extra;
            DirWPtr parent;
            Dir(
                    std::string _name,
                    Type _type,
                    ContentsMap _contents,
                    std::optional<std::variant<OffsetName>> _extra,
                    DirWPtr _parent
            ) :
            name(std::move(_name)), type(_type),
            contents(std::move(_contents)), extra(std::move(_extra)), parent(_parent){

            }
        };

        std::vector<std::string> splitPath(std::string path){
            if(path == "/"){
                return {};
            }
            std::vector<std::string> ret;
            boost::trim_if(path, boost::is_any_of("/"));
            boost::split(ret, path, boost::is_any_of("/"));
            return ret;
        }

        std::string getLast(const std::string &path){
            return splitPath(path).back();
        }

        bool isDir(const std::optional<std::variant<DirPtr, FilePtr>> &var){
            return var && std::holds_alternative<DirPtr>(*var);
        }

        bool isFile(const std::optional<std::variant<DirPtr, FilePtr>> &var){
            return var && std::holds_alternative<FilePtr>(*var);
        }

        std::optional<std::variant<DirPtr, FilePtr>>
                findPath(const std::string &path, uint_fast32_t fromRight = 0){
            auto parts = splitPath(path);
            auto curDir = _rootDir;
            int_fast32_t limit = parts.size() - fromRight;
            if(limit < 0){
                return std::nullopt;
            }
            int_fast32_t i = 0;
            for(const auto &part: parts){
                if(i == limit){
                    break;
                }
                if(curDir->contents.find(part) != curDir->contents.end()){
                    auto var = curDir->contents[part];
                    if(std::holds_alternative<DirPtr>(var)){
                        curDir = std::get<DirPtr>(var);
                    } else {
                        return std::get<FilePtr>(var);
                    }
                } else {
                    return std::nullopt;
                }
                i++;
            }
            return curDir;
        }

        std::string genFileName(const std::string &artist, const std::string &title){
            return escapeName(artist + " - " + title);
        }

        std::string escapeName(std::string str){
            boost::remove_erase_if(str, [] (auto ch0){
                unsigned char ch = ch0;
                return ch <= 31 || std::string("<>:/\\|?*").find(ch) != std::string::npos;
            });
            std::replace(str.begin(), str.end(), '"', '\"');
            return str;
        }

        bool createDirNoLock(const std::string &dirPath){
            auto dirO = findPath(dirPath, 1);
            if(!isDir(dirO)){
                return false;
            }
            auto dirName = getLast(dirPath);
            if(std::get<DirPtr>(*dirO)->type == Dir::Type::ROOT_SEARCH_DIR ) {
                auto parentDir = std::get<DirPtr>(*dirO);
                parentDir->contents.insert(std::make_pair(
                        dirName,
                        std::make_shared<Dir>(
                                dirName, Dir::Type::SEARCH_DIR, ContentsMap{},
                                OffsetName{_numSearchFiles, dirName}, DirWPtr{parentDir}
                        )
                ));
                insertMp3sInDir(parentDir, dirName, dirName, 0, _numSearchFiles);
                return true;
            } else if(std::get<DirPtr>(*dirO)->type == Dir::Type::SEARCH_DIR){
                auto cnt = std::stoul(dirName);
                auto parentDir = std::get<DirPtr>(*dirO);
                auto offset = std::get<OffsetName>(*parentDir->extra).offset;
                auto searchName = std::get<OffsetName>(*parentDir->extra).name;
                parentDir->contents.insert(std::make_pair(
                        dirName,
                        std::make_shared<Dir>(
                                dirName, Dir::Type::SEARCH_DIR, ContentsMap{},
                                OffsetName{offset + cnt, searchName}, DirWPtr{parentDir}
                        )
                ));
                insertMp3sInDir(parentDir, dirName, searchName, offset, cnt);
            }
            return false;
        }

        void insertMp3sInDir(
                const DirPtr &parentDir, const std::string &dirName,
                const std::string &searchName,
                uint_fast32_t offset, uint_fast32_t count
        ){
            auto res = json::parse(_queryMaker->makeSearchQuery(searchName, offset, count));
            auto resp = res["response"];
            auto curDir = std::get<DirPtr>(parentDir->contents[dirName]);
            for (const auto &item: resp["items"]) {
                auto initialFileName = genFileName(item["artist"], item["title"]);
                auto fileName = initialFileName + ".mp3";
                uint_fast32_t i = 0;
                while(curDir->contents.find(fileName) != curDir->contents.end()){
                    fileName = initialFileName + std::to_string(i) + ".mp3";
                    i++;
                }
                curDir->contents.insert(
                        std::make_pair<>(
                                fileName,
                                std::make_shared<File>(
                                        fileName,
                                        File::Type::MUSIC_FILE,
                                        RemoteFile{item["url"], item["owner_id"],
                                                   item["id"], item["artist"], item["title"]},
                                        curDir
                                )
                        )
                );
            }
        }

        std::mutex _fsMutex;
        std::shared_ptr<Dir> _rootDir;
        std::shared_ptr<TQueryMaker> _queryMaker;
        uint_fast32_t _numSearchFiles;
    };
}
