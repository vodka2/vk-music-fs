#pragma once

#include <boost/algorithm/string.hpp>
#include "common_fs.h"
#include "FsPath.h"
#include "IdGenerator.h"
#include "DirOrFile.h"
#include "Dir.h"
#include <mp3core/RemoteFile.h>

namespace vk_music_fs {
    namespace fs {
        class FsUtils {
        public:
            FsUtils();
            bool isRefreshDir(const std::string &dir);
            FsPath findPath(
                    const DirPtr &dir, const std::string &path, uint_fast32_t pathSize = 1
            );
            FsPath findPath(
                    const DirPtr &dir, const std::string &path, const std::vector<FsPath> &locked, uint_fast32_t pathSize = 1
            );
            std::vector<std::string> getEntries(const DirPtr &dir);
            std::vector<std::string> getEntries(const DirPtr &rootDir, const std::string &path, const std::string &fullPath);
            FileOrDirMeta getMeta(const DirPtr &rootDir, const std::string &path);
            std::string stripPathPrefix(std::string path, const std::string &prefix);
            QueryParams parseQuery(const std::string &dirName);
            std::vector<FilePtr> addFilesToDir(
                    const DirPtr &dir, const std::vector<RemoteFile> &files,
                    const std::shared_ptr<IdGenerator> &idGenerator, const std::string &extension
            );
            RemoteFile getRemoteFile(FsPath &path, const std::string &fullPath);
            template <typename TFunc>
            void limitItems(const DirPtr &dir, uint_fast32_t num, TFunc func) {
                std::vector<DirOrFile> items;
                for(const auto &s: dir->getContents()){
                    if(func(s.second)){
                        items.push_back(s.second);
                    }
                }
                if(items.size() >= num) {
                    std::sort(items.begin(), items.end(), [](const auto &a, const auto &b) {
                        return a.getTime() > b.getTime();
                    });
                    for (auto it = items.cbegin(); it != items.begin() + (items.size() - num); it++) {
                        dir->removeItem(it->getName());
                    }
                }
            }
            template <typename TFunc>
            void deleteItems(const DirPtr &dir, TFunc func) {
                auto &contents = dir->getContents();
                for (auto it = contents.begin(); it != contents.end();) {
                    if(func(it->second)) {
                        it = contents.erase(it);
                    } else {
                        it++;
                    }
                }
            }
            auto getAllDeleter(){
                return [](const DirOrFile &item){
                    return !(item.isDir() && item.dir()->getDirExtra() &&
                             std::holds_alternative<OffsetCntRemoteFile>(*item.dir()->getDirExtra()));
                };
            }
            template <typename TExtra>
            auto getCounterDirLeaver(){
                return [](const DirOrFile &item){
                    if(item.isFile()){
                        return true;
                    }
                    DirPtr dir = item.dir();
                    if (dir->getDirExtra() && std::holds_alternative<OffsetCntRemoteFile>(*dir->getDirExtra())) {
                        return false;
                    }
                    auto extra = std::get<TExtra>(*dir->getParent()->getDirExtra());
                    return !(extra.getCounterDir() == dir);
                };
            }

        private:
            std::vector<std::string> splitPath(std::string path);
            bool isPathElementLocked(const DirOrFile &element, const std::vector<FsPath> &locked);
            const uint_fast8_t TWO_SLASHES_LENGTH = 2;
        };
    }
}
