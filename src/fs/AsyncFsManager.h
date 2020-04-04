#pragma once

#include <common/ThreadPool.h>

#include <utility>
#include "common_fs.h"
#include <mp3core/RemoteFile.h>
#include "Dir.h"
#include "IdGenerator.h"
#include "Mp3FileName.h"
#include "FsSettings.h"

namespace vk_music_fs {
    namespace fs {
        template <typename TFsUtils, typename TFileCache, typename TRealFs>
        class AsyncFsManager {
        public:
            AsyncFsManager(
                    std::shared_ptr<TFsUtils> fsUtils,
                    std::shared_ptr<ThreadPool> threadPool,
                    std::shared_ptr<IdGenerator> idGenerator,
                    std::shared_ptr<TFileCache> cache,
                    std::shared_ptr<TRealFs> realFs,
                    std::shared_ptr<FsSettings> fsSettings
            )
            : _fsUtils(std::move(fsUtils)), _threadPool(std::move(threadPool)),
            _idGenerator(std::move(idGenerator)), _cache(std::move(cache)),
            _fsSettings(std::move(fsSettings)), _realFs(realFs) {}

            void createFiles(const DirPtr &dir, const std::vector<RemoteFile> &remoteFiles) {
                auto createdFiles = _fsUtils->addFilesToDir(dir, remoteFiles, _idGenerator, _fsSettings->getMp3Ext());
                if (_fsSettings->isUseAsyncNotifier()) {
                    std::vector<std::string> createdFileNames;
                    for (auto &file: createdFiles) {
                        file->setHidden(true);
                        createdFileNames.push_back(file->getName());
                    }
                    std::string dirPath = dir->getAbsolutePath();
                    _threadPool->post([this, createdFileNames, remoteFiles, dirPath]() {
                        for (uint_fast32_t i = 0; i < remoteFiles.size(); i++) {
                            _cache->getFileSize(remoteFiles[i]);
                            _realFs->createFile(_fsSettings->getPathToFs() + "/" + dirPath + "/" + createdFileNames[i]);
                        }
                    });
                }
            }

        private:
            std::shared_ptr<TFsUtils> _fsUtils;
            std::shared_ptr<ThreadPool> _threadPool;
            std::shared_ptr<IdGenerator> _idGenerator;
            std::shared_ptr<TFileCache> _cache;
            std::shared_ptr<FsSettings> _fsSettings;
            std::shared_ptr<TRealFs> _realFs;
        };
    }
}
