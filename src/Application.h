#pragma once

#include <common.h>

namespace vk_music_fs {
    template <typename TFileManager, typename TVkApi>
    class Application {
    public:
        Application(const std::shared_ptr<TVkApi> &api, const std::shared_ptr<TFileManager> &fileManager)
        :_api(api), _fileManager(fileManager){
        }

        int_fast32_t open(const std::string &filename){
            return _fileManager->open(filename);
        }

        ByteVect read(uint_fast32_t id, uint_fast32_t offset, uint_fast32_t size){
            return _fileManager->read(id, offset, size);
        }

        void close(uint_fast32_t id){
            _fileManager->close(id);
        }

        std::vector<std::string> getEntries(const std::string &dirPath){
            return _api->getEntries(dirPath);
        }

        FileOrDirType getType(const std::string &path){
            return _api->getType(path);
        }

        bool createDir(const std::string &dirPath){
            return _api->createDir(dirPath);
        }

        bool createDummyDir(const std::string &dirPath){
            return _api->createDummyDir(dirPath);
        }

        bool renameDummyDir(const std::string &oldPath, const std::string &newPath){
            return _api->renameDummyDir(oldPath, newPath);
        }

        uint_fast32_t getFileSize(const std::string &path){
            return _fileManager->getFileSize(path);
        }
    private:
        std::shared_ptr<TVkApi> _api;
        std::shared_ptr<TFileManager> _fileManager;
    };
}