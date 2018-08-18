#pragma once

#include <regex>
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
#include "Dir.h"
#include "File.h"
#include "DirOrFile.h"

using json = nlohmann::json;

namespace vk_music_fs {
    namespace fs {
        template<typename TQueryMaker>
        class AudioFs {
        public:
            AudioFs(const std::shared_ptr<TQueryMaker> &queryMaker, const NumSearchFiles &numSearchFiles,
                    const Mp3Extension &ext)
                    : _ext(ext.t), _queryMaker(queryMaker), _numSearchFiles(numSearchFiles),
                      _rootDir(
                              std::make_shared<Dir>("/", Dir::Type::ROOT_DIR, ContentsMap{}, std::nullopt, DirWPtr{})) {
                auto searchDir = std::make_shared<Dir>(
                        "Search", Dir::Type::ROOT_SEARCH_DIR, ContentsMap{}, std::nullopt, _rootDir
                );
                _rootDir->addItem(searchDir);
                auto myAudiosDir = std::make_shared<Dir>(
                        "My audios", Dir::Type::ROOT_MY_AUDIOS_DIR, ContentsMap{}, 0, _rootDir
                );
                _rootDir->addItem(myAudiosDir);
            }

            bool renameDummyDir(const std::string &oldPath, const std::string &newPath) {
                std::scoped_lock<std::mutex> lock(_fsMutex);
                auto oldDirO = findPath(oldPath);
                auto newDirO = findPath(newPath, 1);
                if (
                        isDir(oldDirO) && isDummyDirParent((*oldDirO).dir()->getParent()) &&
                        (*oldDirO).dir()->getType() == Dir::Type::DUMMY_DIR &&
                        isDir(newDirO) &&
                        (*oldDirO).dir()->getParent() == (*newDirO).dir()
                        ) {
                    auto oldDir = (*oldDirO).dir();
                    oldDir->getParent()->removeItem(oldDir->getName());
                    return createDirNoLock(newPath);
                }
                return false;
            }

            bool createDir(const std::string &dirPath) {
                std::scoped_lock<std::mutex> lock(_fsMutex);
                return createDirNoLock(dirPath);
            }

            std::vector<std::string> getEntries(const std::string &dirPath) {
                std::scoped_lock<std::mutex> lock(_fsMutex);
                std::vector<std::string> ret;
                auto dirO = findPath(dirPath);
                if (!isDir(dirO)) {
                    return ret;
                }
                auto dir = (*dirO).dir();
                for (const auto &item : dir->getContents()) {
                    ret.push_back(item.first);
                }
                return ret;
            }

            FileOrDirType getType(const std::string &path) {
                std::scoped_lock<std::mutex> lock(_fsMutex);
                auto pathO = findPath(path);
                if (pathO) {
                    if (isDir(pathO)) {
                        return FileOrDirType::DIR_ENTRY;
                    } else {
                        return FileOrDirType::FILE_ENTRY;
                    }
                }
                return FileOrDirType::NOT_EXISTS;
            }

            RemoteFile getRemoteFile(const std::string &path) {
                std::scoped_lock<std::mutex> lock(_fsMutex);
                auto pathO = findPath(path);
                return (*pathO).file()->getRemFile();
            }

            bool createDummyDir(const std::string &path) {
                std::scoped_lock<std::mutex> lock(_fsMutex);
                auto pathO = findPath(path, 1);
                if (isDir(pathO) && isDummyDirParent((*pathO).dir())) {
                    auto dirName = getLast(path);
                    auto parentDir = (*pathO).dir();
                    parentDir->addItem(
                            std::make_shared<Dir>(
                                    dirName, Dir::Type::DUMMY_DIR, ContentsMap{}, std::nullopt,
                                    DirWPtr{parentDir}
                            )
                 );
                    return true;
                }
                return false;
            }

            bool deleteDir(const std::string &path) {
                std::scoped_lock<std::mutex> lock(_fsMutex);
                auto pathO = findPath(path);
                if (isDir(pathO)
                    &&
                    (
                            (*pathO).dir()->getType() == Dir::Type::SEARCH_DIR ||
                            (*pathO).dir()->getType() == Dir::Type::DUMMY_DIR ||
                            (*pathO).dir()->getType() == Dir::Type::MY_AUDIOS_DIR
                    )
                        ) {
                    (*pathO).dir()->getParent()->removeItem(getLast(path));
                    return true;
                }
                return false;
            }

            bool deleteFile(const std::string &path) {
                std::scoped_lock<std::mutex> lock(_fsMutex);
                auto pathO = findPath(path);
                if (isFile(pathO) && (*pathO).file()->getType() == File::Type::MUSIC_FILE) {
                    (*pathO).file()->getParent()->removeItem(getLast(path));
                    return true;
                }
                return false;
            }

        private:
            bool isDummyDirParent(const DirPtr &ptr){
                return
                    ptr->getType() == Dir::Type::ROOT_SEARCH_DIR ||
                    ptr->getType() == Dir::Type::SEARCH_DIR ||
                    ptr->getType() == Dir::Type::ROOT_MY_AUDIOS_DIR ||
                    ptr->getType() == Dir::Type::MY_AUDIOS_DIR
                ;
            }
            bool isDir(const std::optional<DirOrFile> &opt){
                return opt && (*opt).isDir();
            }
            bool isFile(const std::optional<DirOrFile> &opt){
                return opt && (*opt).isFile();
            }
            std::vector<std::string> splitPath(std::string path) {
                if (path == "/") {
                    return {};
                }
                std::vector<std::string> ret;
                boost::trim_if(path, boost::is_any_of("/"));
                boost::split(ret, path, boost::is_any_of("/"));
                return ret;
            }

            std::string getLast(const std::string &path) {
                return splitPath(path).back();
            }

            std::optional<DirOrFile>
            findPath(const std::string &path, uint_fast32_t fromRight = 0) {
                auto parts = splitPath(path);
                auto curDir = _rootDir;
                int_fast32_t limit = parts.size() - fromRight;
                if (limit < 0) {
                    return std::nullopt;
                }
                int_fast32_t i = 0;
                for (const auto &part: parts) {
                    if (i == limit) {
                        break;
                    }
                    if (curDir->hasItem(part)) {
                        auto var = curDir->getItem(part);
                        if (isDir(var)) {
                            curDir = var.dir();
                        } else {
                            return var.file();
                        }
                    } else {
                        return std::nullopt;
                    }
                    i++;
                }
                return curDir;
            }

            std::string genFileName(const std::string &artist, const std::string &title) {
                return escapeName(artist + " - " + title);
            }

            std::string escapeName(std::string str) {
                boost::remove_erase_if(str, [](auto ch0) {
                    unsigned char ch = ch0;
                    return ch <= 31 || std::string("<>:/\\|?*").find(ch) != std::string::npos;
                });
                std::replace(str.begin(), str.end(), '"', '\"');
                return str;
            }

            bool createDirNoLock(const std::string &dirPath) {
                auto dirO = findPath(dirPath, 1);
                if (!isDir(dirO)) {
                    return false;
                }
                auto dirName = getLast(dirPath);
                auto type = (*dirO).dir()->getType();
                if (type == Dir::Type::ROOT_SEARCH_DIR) {
                    auto parentDir = (*dirO).dir();
                    parentDir->addItem(
                            std::make_shared<Dir>(
                                    dirName, Dir::Type::SEARCH_DIR, ContentsMap{},
                                    OffsetName{_numSearchFiles, dirName}, DirWPtr{parentDir}
                            )
                    );
                    insertMp3sInDir(parentDir, dirName, makeSearchQuery(dirName, 0, _numSearchFiles));
                    return true;
                } else if (type == Dir::Type::SEARCH_DIR) {
                    auto parentDir = (*dirO).dir();
                    std::string searchName;
                    uint_fast32_t offset;
                    uint_fast32_t cnt;
                    auto queryParams = parseQuery(dirName);
                    if(queryParams.type != QueryParams::Type::STRING){
                        searchName = parentDir->getOffsetName().getName();
                        if(queryParams.type == QueryParams::Type::TWO_NUMBERS) {
                            offset = queryParams.first;
                            cnt = queryParams.second;
                        } else {
                            offset = parentDir->getOffsetName().getOffset();
                            cnt =  queryParams.first;
                        }
                    } else {
                        offset = 0;
                        cnt = _numSearchFiles;
                        searchName = parentDir->getOffsetName().getName() + " " + dirName;
                    }
                    parentDir->addItem(
                            std::make_shared<Dir>(
                                    dirName, Dir::Type::SEARCH_DIR, ContentsMap{},
                                    OffsetName{offset + cnt, searchName}, DirWPtr{parentDir}
                            )
                    );
                    insertMp3sInDir(parentDir, dirName, makeSearchQuery(searchName, offset, cnt));
                    return true;
                } else if(type == Dir::Type::ROOT_MY_AUDIOS_DIR || type == Dir::Type::MY_AUDIOS_DIR){
                    auto parentDir = (*dirO).dir();
                    std::smatch mtc;
                    uint_fast32_t offset;
                    uint_fast32_t cnt;
                    auto queryParams = parseQuery(dirName);
                    if(queryParams.type != QueryParams::Type::STRING){
                        if(queryParams.type == QueryParams::Type::TWO_NUMBERS) {
                            offset = queryParams.first;
                            cnt = queryParams.second;
                        } else {
                            offset = parentDir->getNumber();
                            cnt =  queryParams.first;
                        }
                        parentDir->addItem(
                                std::make_shared<Dir>(
                                        dirName, Dir::Type::MY_AUDIOS_DIR, ContentsMap{},
                                        offset + cnt, DirWPtr{parentDir}
                                )
                        );
                        insertMp3sInDir(parentDir, dirName, makeMyAudiosQuery(offset, cnt));
                        return true;
                    } else {
                        return false;
                    }
                }
                return false;
            }

            struct QueryParams{
                enum class Type{
                    TWO_NUMBERS,
                    ONE_NUMBER,
                    STRING
                } type;
                uint_fast32_t first;
                uint_fast32_t second;
                QueryParams(
                        Type type,
                        uint_fast32_t first,
                        uint_fast32_t second
                ): type(type), first(first), second(second){
                }
            };

            QueryParams parseQuery(const std::string &dirName){
                std::regex offsetRegex("^([0-9]{1,6})(?:-([0-9]{1,6}))?$");
                std::smatch mtc;
                if(std::regex_search(dirName, mtc, offsetRegex)){
                    if(mtc[2].matched) {
                        return QueryParams(
                                QueryParams::Type::TWO_NUMBERS, std::stoul(mtc[1].str()), std::stoul(mtc[2].str())
                        );
                    } else {
                        return QueryParams(QueryParams::Type::ONE_NUMBER, std::stoul(mtc[1].str()), 0);
                    }
                } else {
                    return QueryParams(QueryParams::Type::STRING, 0, 0);
                }
            }

            json makeSearchQuery(
                    const std::string &searchName,
                    uint_fast32_t offset, uint_fast32_t count
            ){
                auto res = json::parse(_queryMaker->makeSearchQuery(searchName, offset, count));
                return std::move(res);
            }

            json makeMyAudiosQuery(
                    uint_fast32_t offset, uint_fast32_t count
            ){
                auto res = json::parse(_queryMaker->makeMyAudiosQuery(offset, count));
                return std::move(res);
            }

            void insertMp3sInDir(
                    const DirPtr &parentDir, const std::string &dirName, json returnedJson
            ) {
                auto resp = returnedJson["response"];
                auto curDir = parentDir->getItem(dirName).dir();
                for (const auto &item: resp["items"]) {
                    auto initialFileName = genFileName(item["artist"], item["title"]);
                    auto fileName = initialFileName + _ext;
                    uint_fast32_t i = 2;
                    while (curDir->hasItem("fileName")) {
                        fileName = initialFileName + "_" + std::to_string(i) + _ext;
                        i++;
                    }
                    curDir->addItem(
                        std::make_shared<File>(
                                fileName,
                                File::Type::MUSIC_FILE,
                                RemoteFile{item["url"], item["owner_id"],
                                           item["id"], item["artist"], item["title"]},
                                curDir
                        )
                    );
                }
            }

            std::mutex _fsMutex;
            std::shared_ptr<Dir> _rootDir;
            std::string _ext;
            std::shared_ptr<TQueryMaker> _queryMaker;
            uint_fast32_t _numSearchFiles;
        };
    }
    template <typename T>
    using AudioFs = fs::AudioFs<T>;
}
