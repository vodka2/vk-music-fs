#pragma once

#include <boost/algorithm/string.hpp>
#include "common_fs.h"
#include "FsPath.h"
#include "IdGenerator.h"
#include <RemoteFile.h>

namespace vk_music_fs {
    namespace fs {
        class FsUtils {
        public:
            FsUtils();
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
            void addFilesToDir(
                    const DirPtr &dir, const std::vector<RemoteFile> &files,
                    const std::shared_ptr<IdGenerator> &idGenerator, const std::string &extension
            );
            RemoteFile getRemoteFile(FsPath &path, const std::string &fullPath);
            void deleteAllFiles(const DirPtr &dir);
            void limitFiles(const DirPtr &dir, uint_fast32_t num);

        private:
            std::vector<std::string> splitPath(std::string path);
            bool isPathElementLocked(const DirOrFile &element, const std::vector<FsPath> &locked);
            const uint_fast8_t TWO_SLASHES_LENGTH = 2;
        };
    }
}
