#pragma once

#include <common.h>
#include "ProgramOptions.h"
#include "MusicFsException.h"

namespace vk_music_fs {
    template <typename TFileManager, typename TAudioFs, typename TLogger>
    class Application {
    public:
        Application(
                const std::shared_ptr<TAudioFs> &api, const std::shared_ptr<TFileManager> &fileManager,
                const std::shared_ptr<TLogger> &logger,
                std::shared_ptr<ProgramOptions> options
        )
        :_options(options), _audioFs(api), _fileManager(fileManager), _logger(logger){
        }

        int_fast32_t open(const std::string &filename){
            try {
                return _fileManager->open(filename);
            } catch (const MusicFsException &exc){
                _logger->logException(exc);
                throw;
            }
        }

        ByteVect read(uint_fast32_t id, uint_fast32_t offset, uint_fast32_t size){
            try {
                return _fileManager->read(id, offset, size);
            } catch (const MusicFsException &exc){
                _logger->logException(exc);
                throw;
            }
        }

        void close(uint_fast32_t id){
            try {
                _fileManager->close(id);
            } catch (const MusicFsException &exc){
                _logger->logException(exc);
                throw;
            }
        }

        std::vector<std::string> getEntries(const std::string &dirPath){
            try {
                return _audioFs->getEntries(dirPath);
            } catch (const MusicFsException &exc){
                _logger->logException(exc);
                throw;
            }
        }

        FileOrDirMeta getMeta(const std::string &path){
            try {
                return _audioFs->getMeta(path);
            } catch (const MusicFsException &exc){
                _logger->logException(exc);
                throw;
            }
        }

        bool createDir(const std::string &dirPath){
            try {
                return _audioFs->createDir(dirPath);
            } catch (const MusicFsException &exc){
                _logger->logException(exc);
                throw;
            }
        }

        bool renameDir(const std::string &oldPath, const std::string &newPath){
            try {
                return _audioFs->renameDir(oldPath, newPath);
            } catch (const MusicFsException &exc){
                _logger->logException(exc);
                throw;
            }
        }

        uint_fast32_t getFileSize(const std::string &path){
            try {
                return _fileManager->getFileSize(path);
            } catch (const MusicFsException &exc){
                _logger->logException(exc);
                throw;
            }
        }

        bool deleteDir(const std::string &path){
            try {
                return _audioFs->deleteDir(path);
            } catch (const MusicFsException &exc){
                _logger->logException(exc);
                throw;
            }
        }

        bool deleteFile(const std::string &path){
            try {
                return _audioFs->deleteFile(path);
            } catch (const MusicFsException &exc){
                _logger->logException(exc);
                throw;
            }
        }
    private:
        std::shared_ptr<ProgramOptions> _options;
        std::shared_ptr<TAudioFs> _audioFs;
        std::shared_ptr<TFileManager> _fileManager;
        std::shared_ptr<TLogger> _logger;
    };
}