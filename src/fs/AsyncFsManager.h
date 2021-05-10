#pragma once

#include <utility>
#include "common_fs.h"
#include <mp3core/RemoteFile.h>
#include "Dir.h"
#include "File.h"
#include <common/RemotePhotoFile.h>
#include "common/IdGenerator.h"
#include "Mp3FileName.h"
#include "FsSettings.h"

namespace vk_music_fs {
    namespace fs {
        template <typename TFsUtils, typename TFileCache, typename TPhotoCache, typename TRealFs, typename TPool>
        class AsyncFsManager {
        public:
            AsyncFsManager(
                    std::shared_ptr<TFsUtils> fsUtils,
                    std::shared_ptr<TPool> threadPool,
                    std::shared_ptr<IdGenerator> idGenerator,
                    std::shared_ptr<TFileCache> fileCache,
                    std::shared_ptr<TPhotoCache> photoCache,
                    std::shared_ptr<TRealFs> realFs,
                    std::shared_ptr<FsSettings> fsSettings
            )
            : _fsUtils(std::move(fsUtils)), _threadPool(std::move(threadPool)),
              _idGenerator(std::move(idGenerator)), _fileCache(std::move(fileCache)), _photoCache(std::move(photoCache)),
              _fsSettings(std::move(fsSettings)), _realFs(realFs) {}

            void createFiles(const DirPtr &dir, const std::vector<RemoteFile> &remoteFiles) {
                auto createdFiles = _fsUtils->addFilesToDir(dir, remoteFiles, _idGenerator, _fsSettings->getMp3Ext());
                if (_fsSettings->isUseAsyncNotifier()) {
                    std::vector<std::string> createdFileNames;
                    std::vector<RemoteFile> createdRemoteFiles;
                    createdFileNames.reserve(createdFiles.size());
                    createdRemoteFiles.reserve(createdFiles.size());
                    for (auto &file: createdFiles) {
                        file->setHidden(true);
                        createdFileNames.push_back(file->getName());
                        createdRemoteFiles.push_back(std::get<RemoteFile>(*file->getExtra()));
                    }
                    std::string dirPath = dir->getAbsolutePath();
                    _threadPool->post([this, createdFileNames, createdRemoteFiles, dirPath]() {
                        for (uint_fast32_t i = 0; i < createdRemoteFiles.size(); i++) {
                            _fileCache->getFileSize(createdRemoteFiles[i]);
                            _realFs->createFile(_fsSettings->getPathToFs() + "/" + dirPath + "/" + createdFileNames[i]);
                        }
                    });
                }
            }

            void createPhoto(const DirPtr &dir, const RemotePhotoFile &url) {
                if (dir->hasItem(_fsSettings->getPhotoName())) {
                    return;
                }
                auto file = std::make_shared<File>(
                        _fsSettings->getPhotoName(), _idGenerator->getNextId(), dir->getNumFiles() + 1,
                        url, dir
                );
                dir->addItem(file);
                if (_fsSettings->isUseAsyncNotifier()) {
                    file->setHidden(true);
                    std::string dirPath = dir->getAbsolutePath();
                    _threadPool->post([this, url, dirPath]() {
                        _photoCache->getFileSize(url);
                        _realFs->createFile(_fsSettings->getPathToFs() + "/" + dirPath + "/" + _fsSettings->getPhotoName());
                    });
                }
            }

        private:
            std::shared_ptr<TFsUtils> _fsUtils;
            std::shared_ptr<TPool> _threadPool;
            std::shared_ptr<IdGenerator> _idGenerator;
            std::shared_ptr<TFileCache> _fileCache;
            std::shared_ptr<TPhotoCache> _photoCache;
            std::shared_ptr<FsSettings> _fsSettings;
            std::shared_ptr<TRealFs> _realFs;
        };
    }
}
