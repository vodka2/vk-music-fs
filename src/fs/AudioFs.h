#pragma once

#include <utility>
#include <common/common.h>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <mp3core/RemoteFile.h>
#include <mutex>
#include <net/HttpException.h>
#include "ctrl/ctrl.h"
#include "Dir.h"
#include "File.h"
#include "DirOrFile.h"
#include "FsException.h"
#include "IdGenerator.h"

namespace vk_music_fs {
    namespace fs {
        template<typename TCtrlTuple>
        class AudioFs {
        public:

            AudioFs(const TCtrlTuple &tuple, const std::shared_ptr<IdGenerator> &idGen)
                : _ctrlTuple(tuple), _rootDir(std::make_shared<Dir>("/", idGen->getNextId(), std::nullopt, DirWPtr{})){
                for_each(_ctrlTuple, [this] (auto &&arg){
                    arg->setRootDir(_rootDir);
                    return false;
                });
            }

            void createDir(const std::string &dirPath) {
                try {
                    for_each(_ctrlTuple, [dirPath](auto &&arg) {
                        if (arg->supports(dirPath)) {
                            arg->createDir(dirPath);
                            return true;
                        }
                        return false;
                    });
                } catch (const MusicFsException &ex){
                    throw FsException(std::string("Error creating directory. ") + ex.what());
                }
            }

            void createFile(const std::string &fileName) {
                try {
                    for_each(_ctrlTuple, [fileName](auto &&arg) {
                        if (arg->supports(fileName)) {
                            arg->createFile(fileName);
                            return true;
                        }
                        return false;
                    });
                } catch (const MusicFsException &ex){
                    throw FsException(std::string("Error creating file. ") + ex.what());
                }
            }

            void rename(const std::string &oldPath, const std::string &newPath){
                try {
                    for_each(_ctrlTuple, [oldPath, newPath](auto &&arg) {
                        if (arg->supports(newPath)) {
                            arg->rename(oldPath, newPath);
                            return true;
                        }
                        return false;
                    });
                } catch (const MusicFsException &ex){
                    throw FsException(std::string("Error renaming directory. ") + ex.what());
                }
            }

            std::vector<std::string> getEntries(const std::string &dirPath) {
                std::optional<std::vector<std::string>> ret;
                try {
                    for_each(_ctrlTuple, [dirPath, &ret](auto &&arg) {
                        if (arg->supports(dirPath)) {
                            ret = arg->getEntries(dirPath);
                            return true;
                        }
                        return false;
                    });
                } catch (const MusicFsException &ex){
                    throw FsException(std::string("Error getting entries. ") + ex.what());
                }
                return *ret;
            }

            FileOrDirMeta getMeta(const std::string &path) {
                std::optional<FileOrDirMeta> ret;
                try {
                    for_each(_ctrlTuple, [path, &ret](auto &&arg) {
                        if (arg->supports(path)) {
                            ret = arg->getMeta(path);
                            return true;
                        }
                        return false;
                    });
                } catch (const MusicFsException &ex){
                    throw FsException(std::string("Error getting meta. ") + ex.what());
                }
                return *ret;
            }

            void deleteDir(const std::string &path) {
                try {
                    for_each(_ctrlTuple, [path](auto &&arg) {
                        if (arg->supports(path)) {
                            arg->deleteDir(path);
                            return true;
                        }
                        return false;
                    });
                } catch (const MusicFsException &ex){
                    throw FsException(std::string("Error deleting directory. ") + ex.what());
                }
            }

            void deleteFile(const std::string &path) {
                try {
                    for_each(_ctrlTuple, [path] (auto &&arg){
                        if(arg->supports(path)){
                            arg->deleteFile(path);
                            return true;
                        }
                        return false;
                    });
                } catch (const MusicFsException &ex){
                    throw FsException(std::string("Error deleting file. ") + ex.what());
                }
            }

            std::vector<FileInfo> getFileInfos(const std::string &dirname){
                std::optional<std::vector<FileInfo>> ret;
                try {
                    for_each(_ctrlTuple, [dirname, &ret] (auto &&arg){
                        if(arg->supports(dirname)){
                            ret = arg->getFileInfos(dirname);
                            return true;
                        }
                        return false;
                    });
                } catch (const MusicFsException &ex){
                    throw FsException(std::string("Error opening file. ") + ex.what());
                }
                return *ret;
            }

            int_fast32_t open(const std::string &filename){
                std::optional<int_fast32_t> ret;
                try {
                    for_each(_ctrlTuple, [filename, &ret] (auto &&arg){
                        if(arg->supports(filename)){
                            ret = arg->open(filename);
                            return true;
                        }
                        return false;
                    });
                } catch (const MusicFsException &ex){
                    throw FsException(std::string("Error opening file. ") + ex.what());
                }
                return *ret;
            }

            uint_fast32_t getFileSize(const std::string &filename){
                std::optional<uint_fast32_t> ret;
                try {
                    for_each(_ctrlTuple, [filename, &ret] (auto &&arg){
                        if(arg->supports(filename)){
                            ret = arg->getFileSize(filename);
                            return true;
                        }
                        return false;
                    });
                } catch (const MusicFsException &ex){
                    throw FsException(std::string("Error getting file size. ") + ex.what());
                }
                return *ret;
            }

        private:
            TCtrlTuple _ctrlTuple;
            std::shared_ptr<Dir> _rootDir;
        };
    }
    template <typename T>
    using AudioFs = fs::AudioFs<T>;
}
