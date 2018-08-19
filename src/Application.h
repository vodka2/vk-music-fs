#pragma once

#include <common.h>

namespace vk_music_fs {
    template <typename TFileManager, typename TAudioFs>
    class Application {
    public:
        Application(const std::shared_ptr<TAudioFs> &api, const std::shared_ptr<TFileManager> &fileManager)
        :_audioFs(api), _fileManager(fileManager){
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
            return _audioFs->getEntries(dirPath);
        }

        FileOrDirMeta getMeta(const std::string &path){
            return _audioFs->getMeta(path);
        }

        bool createDir(const std::string &dirPath){
            return _audioFs->createDir(dirPath);
        }

        bool createDummyDir(const std::string &dirPath){
            return _audioFs->createDummyDir(dirPath);
        }

        bool renameDummyDir(const std::string &oldPath, const std::string &newPath){
            return _audioFs->renameDummyDir(oldPath, newPath);
        }

        uint_fast32_t getFileSize(const std::string &path){
            return _fileManager->getFileSize(path);
        }

        bool deleteDir(const std::string &path){
            return _audioFs->deleteDir(path);
        }

        bool deleteFile(const std::string &path){
            return _audioFs->deleteFile(path);
        }
    private:
        std::shared_ptr<TAudioFs> _audioFs;
        std::shared_ptr<TFileManager> _fileManager;
    };
}