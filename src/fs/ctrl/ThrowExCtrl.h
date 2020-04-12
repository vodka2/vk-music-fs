#pragma once

#include <fs/common_fs.h>
#include <fs/FsException.h>
#include <fs/FsPath.h>

namespace vk_music_fs {
    namespace fs {
        class ThrowExCtrl {
        public:
            ThrowExCtrl() = default;

            void setRootDir(const DirPtr &dir) {}

            bool supports(const std::string &path) { return false; }

            void createDir(const std::string &dirPath) {
                throw FsException("Can't create dir - not supported");
            }

            void createDir(FsPath &path) {
                throw FsException("Can't create dir - not supported");
            }

            void createFile(const std::string &file) {
                throw FsException("Can't create file - not supported");
            }

            void rename(const std::string &oldPath, const std::string &newPath) {
                throw FsException("Can't rename - not supported");
            }

            void rename(FsPath& oldPath, FsPath &newPath) {
                throw FsException("Can't rename - not supported");
            }

            std::vector<std::string> getEntries(const std::string &path) {
                throw FsException("Can't get entries - not supported");
            }

            FileOrDirMeta getMeta(const std::string &path) {
                throw FsException("Can't get file meta - not supported");
            }

            void deleteDir(const std::string &path) {
                throw FsException("Can't delete dir - not supported");
            }

            void deleteFile(const std::string &path) {
                throw FsException("Can't delete file - not supported");
            }

            int_fast32_t open(const std::string &filename) {
                throw FsException("Can't open file - not supported");
            }

            uint_fast32_t getFileSize(const std::string &filename) {
                throw FsException("Can't get file size - not supported");
            }
        };
    }
}