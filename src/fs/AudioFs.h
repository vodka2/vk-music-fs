#pragma once

#include <utility>
#include <common.h>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <RemoteFile.h>
#include <mutex>
#include <net/HttpException.h>
#include "Dir.h"
#include "File.h"
#include "DirOrFile.h"
#include "SearchDirMaker.h"
#include "FsException.h"

namespace vk_music_fs {
    namespace fs {
        template<typename TQueryMaker>
        class AudioFs {
        public:
            AudioFs(const std::shared_ptr<TQueryMaker> &queryMaker, const NumSearchFiles &numSearchFiles,
                    const Mp3Extension &ext, const CreateDummyDirs &createDummyDirs)
                    : _searchDirMaker(queryMaker, numSearchFiles, ext), _createDummyDirs(createDummyDirs.t),
                      _rootDir(std::make_shared<Dir>("/", Dir::Type::ROOT_DIR, std::nullopt, DirWPtr{})) {
                auto searchDir = std::make_shared<Dir>(
                        "Search", Dir::Type::ROOT_SEARCH_DIR, std::nullopt, _rootDir
                );
                _rootDir->addItem(searchDir);
                auto myAudiosDir = std::make_shared<Dir>(
                        "My audios", Dir::Type::ROOT_MY_AUDIOS_DIR, OffsetCnt{0, 0}, _rootDir
                );
                _rootDir->addItem(myAudiosDir);
            }

            bool createDir(const std::string &dirPath) {
                std::scoped_lock<std::mutex> lock(_fsMutex);
                if(_createDummyDirs){
                    return createDummyDir(dirPath);
                } else {
                    return createDirNoLock(dirPath);
                }
            }

            bool renameDir(const std::string &oldPath, const std::string &newPath){
                std::scoped_lock<std::mutex> lock(_fsMutex);
                if(_createDummyDirs){
                    return renameDummyDir(oldPath, newPath);
                }
                return false;
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

            FileOrDirMeta getMeta(const std::string &path) {
                std::scoped_lock<std::mutex> lock(_fsMutex);
                auto pathO = findPath(path);
                if (pathO) {
                    if (isDir(pathO)) {
                        return {FileOrDirMeta::Type::DIR_ENTRY, 0};
                    } else {
                        return {FileOrDirMeta::Type::FILE_ENTRY, (*pathO).file()->getTime()};
                    }
                }
                return {FileOrDirMeta::Type::NOT_EXISTS, 0};
            }

            RemoteFile getRemoteFile(const std::string &path) {
                std::scoped_lock<std::mutex> lock(_fsMutex);
                auto pathO = findPath(path);
                return (*pathO).file()->getRemFile();
            }

            bool deleteDir(const std::string &path) {
                std::scoped_lock<std::mutex> lock(_fsMutex);
                auto pathO = findPath(path);
                if(!isDir(pathO)){
                    return false;
                }
                if (
                        (*pathO).dir()->getType() == Dir::Type::SEARCH_DIR ||
                        (*pathO).dir()->getType() == Dir::Type::DUMMY_DIR ||
                        (*pathO).dir()->getType() == Dir::Type::COUNTER_DIR
                ) {
                    if((*pathO).dir()->getType() == Dir::Type::COUNTER_DIR) {
                        (*pathO).dir()->getParent()->clearContentsExceptNested();
                    } else {
                        (*pathO).dir()->getParent()->removeItem(getLast(path));
                    }
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
            bool createDummyDir(const std::string &path) {
                auto pathO = findPath(path, 1);
                if (isDir(pathO) && isDummyDirParent((*pathO).dir())) {
                    auto dirName = getLast(path);
                    auto parentDir = (*pathO).dir();
                    parentDir->addItem(
                            std::make_shared<Dir>(
                                    dirName, Dir::Type::DUMMY_DIR, std::nullopt,
                                    DirWPtr{parentDir}
                            )
                    );
                    return true;
                }
                return false;
            }

            bool renameDummyDir(const std::string &oldPath, const std::string &newPath) {
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

            bool createDirNoLock(const std::string &dirPath) {
                auto dirO = findPath(dirPath, 1);
                if (!isDir(dirO)) {
                    return false;
                }
                auto dirName = getLast(dirPath);
                auto type = (*dirO).dir()->getType();
                if (type == Dir::Type::ROOT_SEARCH_DIR) {
                    try {
                        auto parentDir = (*dirO).dir();
                        return _searchDirMaker.createSearchDirInRoot(parentDir, dirName);
                    } catch (const MusicFsException &ex){
                        throw FsException("Error creating search directory "+ dirName + " in root. " + ex.what());
                    }
                } else if (type == Dir::Type::SEARCH_DIR) {
                    try{
                        auto parentDir = (*dirO).dir();
                        return _searchDirMaker.createSearchDir(parentDir, dirName);
                    } catch (const MusicFsException &ex){
                        throw FsException("Error creating search directory "+ dirName + ". " + ex.what());
                    }
                } else if(type == Dir::Type::ROOT_MY_AUDIOS_DIR){
                    try {
                        auto parentDir = (*dirO).dir();
                        return _searchDirMaker.createMyAudiosDir(parentDir, dirName);
                    } catch (const MusicFsException &ex){
                        throw FsException("Error creating my audios directory "+ dirName + ". " + ex.what());
                    }
                }
                return false;
            }

            bool isDummyDirParent(const DirPtr &ptr){
                return
                    ptr->getType() == Dir::Type::ROOT_SEARCH_DIR ||
                    ptr->getType() == Dir::Type::SEARCH_DIR ||
                    ptr->getType() == Dir::Type::ROOT_MY_AUDIOS_DIR
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

            bool _createDummyDirs;
            std::mutex _fsMutex;
            std::shared_ptr<Dir> _rootDir;
            SearchDirMaker<TQueryMaker> _searchDirMaker;
        };
    }
    template <typename T>
    using AudioFs = fs::AudioFs<T>;
}
