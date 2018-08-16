#include <utility>

#pragma once

#include <common.h>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <RemoteFile.h>
#include <json.hpp>
#include <boost/algorithm/string.hpp>
#include <regex>

using json = nlohmann::json;

namespace vk_music_fs {
    template <typename TQueryMaker>
    class VkApi {
    public:
        VkApi(const std::shared_ptr<TQueryMaker> &queryMaker, const NumSearchFiles &numSearchFiles)
        : _queryMaker(queryMaker), _numSearchFiles(numSearchFiles),
          _rootDir(std::make_shared<Dir>("/", Dir::Type::ROOT_DIR, ContentsMap{}, DirWPtr{})){
            std::shared_ptr<Dir> searchDir = std::make_shared<Dir>("Search", Dir::Type::ROOT_SEARCH_DIR, ContentsMap{}, _rootDir);
            _rootDir->contents.insert(std::make_pair<>(std::string("Search"), searchDir));
        }

        bool renameDummyDir(const std::string &oldPath, const std::string &newPath){
            auto oldDirO = findPath(oldPath);
            auto newDirO = findPath(newPath, 1);
            if(
                    isDir(oldDirO) &&
                    std::get<DirPtr>(*oldDirO)->parent.lock()->type == Dir::Type::ROOT_SEARCH_DIR &&
                    std::get<DirPtr>(*oldDirO)->type == Dir::Type::DUMMY_DIR &&
                    isDir(newDirO) &&
                    std::get<DirPtr>(*oldDirO)->parent.lock() == std::get<DirPtr>(*newDirO)
            ){
                auto oldDir = std::get<DirPtr>(*oldDirO);
                oldDir->parent.lock()->contents.erase(oldDir->name);
                createDir(newPath);
            }
            return false;
        }

        bool createDir(const std::string &dirPath){
            auto dirO = findPath(dirPath, 1);
            if(isDir(dirO) && std::get<DirPtr>(*dirO)->type == Dir::Type::ROOT_SEARCH_DIR ) {
                auto dirName = getLast(dirPath);
                auto parentDir = std::get<DirPtr>(*dirO);
                auto res = json::parse(_queryMaker->makeSearchQuery(dirName, _numSearchFiles));
                auto resp = res["response"];
                parentDir->contents.insert(std::make_pair(
                        dirName,
                        std::make_shared<Dir>(dirName, Dir::Type::SEARCH_DIR, ContentsMap{}, DirWPtr{parentDir})
                ));
                auto curDir = std::get<DirPtr>(parentDir->contents[dirName]);
                for (const auto &item: resp["items"]) {
                    auto fileName = genFileName(item["artist"], item["title"]);
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
                return true;
            }
            return false;
        }

        std::vector<std::string> getEntries(const std::string &dirPath){
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
            auto pathO = findPath(path);
            return std::get<RemoteFile>(std::get<FilePtr>(*pathO)->contents);
        }

        bool createDummyDir(const std::string &path){
            auto pathO = findPath(path, 1);
            if(isDir(pathO)){
                auto dirName = getLast(path);
                auto parentDir = std::get<DirPtr>(*pathO);
                parentDir->contents.insert(std::make_pair(
                        dirName,
                        std::make_shared<Dir>(dirName, Dir::Type::DUMMY_DIR, ContentsMap{}, DirWPtr{parentDir})
                ));
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

        struct Dir{
            std::string name;
            enum class Type{
                SEARCH_DIR,
                DUMMY_DIR,
                ROOT_DIR,
                ROOT_SEARCH_DIR
            } type;
            ContentsMap contents;
            DirWPtr parent;
            Dir(
                    std::string _name,
                    Type _type,
                    ContentsMap _contents,
                    DirWPtr _parent
            ) : name(std::move(_name)), type(_type), contents(std::move(_contents)), parent(_parent){

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
            return artist + " - " + title + ".mp3";
        }

        std::shared_ptr<Dir> _rootDir;
        std::shared_ptr<TQueryMaker> _queryMaker;
        uint_fast32_t _numSearchFiles;
    };
}
