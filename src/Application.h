#pragma once

#include <common.h>
#include <net/HttpStreamCommon.h>
#include "ProgramOptions.h"
#include "MusicFsException.h"

namespace vk_music_fs {
    template <typename TFileManager, typename TAudioFs, typename TLogger, typename THttpCommon>
    class Application {
    public:
        Application(
                const std::shared_ptr<TAudioFs> &api, const std::shared_ptr<TFileManager> &fileManager,
                const std::shared_ptr<THttpCommon> &httpCommon,
                const std::shared_ptr<TLogger> &logger,
                std::shared_ptr<ProgramOptions> options
        )
        :
        _httpCommon(httpCommon), _options(options),
        _audioFs(api), _fileManager(fileManager), _logger(logger){
        }

        ~Application(){
            _httpCommon->stop();
        }

        int_fast32_t open(const std::string &filename){
            try {
                return _audioFs->open(filename);
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

        void createDir(const std::string &dirPath){
            try {
                _audioFs->createDir(dirPath);
            } catch (const MusicFsException &exc){
                _logger->logException(exc);
                throw;
            }
        }

        void rename(const std::string &oldPath, const std::string &newPath){
            try {
                _audioFs->rename(oldPath, newPath);
            } catch (const MusicFsException &exc){
                _logger->logException(exc);
                throw;
            }
        }

        uint_fast32_t getFileSize(const std::string &path){
            try {
                return _audioFs->getFileSize(path);
            } catch (const MusicFsException &exc){
                _logger->logException(exc);
                throw;
            }
        }

        void deleteDir(const std::string &path){
            try {
                _audioFs->deleteDir(path);
            } catch (const MusicFsException &exc){
                _logger->logException(exc);
                throw;
            }
        }

        void deleteFile(const std::string &path){
            try {
                _audioFs->deleteFile(path);
            } catch (const MusicFsException &exc){
                _logger->logException(exc);
                throw;
            }
        }
    private:
        std::shared_ptr<THttpCommon> _httpCommon;
        std::shared_ptr<ProgramOptions> _options;
        std::shared_ptr<TAudioFs> _audioFs;
        std::shared_ptr<TFileManager> _fileManager;
        std::shared_ptr<TLogger> _logger;
    };
}