#pragma once

#include <fs/common_fs.h>
#include <fs/FsPath.h>
#include "RedirectCtrl.h"

namespace vk_music_fs {
    namespace fs {
        template <typename TCtrl, typename TFsUtils, typename TFileManager>
        class RemoteFileWrapper: public RedirectCtrl<TCtrl> {
        public:
            RemoteFileWrapper(
                    std::shared_ptr<TCtrl> ctrl,
                    std::shared_ptr<TFsUtils> fsUtils,
                    std::shared_ptr<TFileManager> fileManager
            ) : RedirectCtrl<TCtrl>(ctrl), _ctrl(ctrl), _fsUtils(fsUtils), _fileManager(fileManager){
            }

            std::string transformPath(const std::string &path){
                return _ctrl->transformPath(path);
            }

            int_fast32_t open(const std::string &filename){
                FsPath fsPath = _fsUtils->findPath(_ctrl->getCtrlDir(), _ctrl->transformPath(filename));
                FsPathUnlocker unlocker{fsPath};
                auto remoteFile = _fsUtils->getRemoteFile(fsPath, filename);
                return _fileManager->open(remoteFile, filename);
            }

            uint_fast32_t getFileSize(const std::string &filename){
                FsPath fsPath = _fsUtils->findPath(_ctrl->getCtrlDir(), _ctrl->transformPath(filename));
                FsPathUnlocker unlocker{fsPath};
                auto remoteFile = _fsUtils->getRemoteFile(fsPath, filename);
                return _fileManager->getFileSize(remoteFile, filename);
            }

            std::vector<std::string> getEntries(const std::string &path) {
                return _fsUtils->getEntries(_ctrl->getCtrlDir(), _ctrl->transformPath(path), path);
            }

            FileOrDirMeta getMeta(const std::string &path) {
                return _fsUtils->getMeta(_ctrl->getCtrlDir(), _ctrl->transformPath(path));
            }

        private:
            std::shared_ptr<TCtrl> _ctrl;
            std::shared_ptr<TFsUtils> _fsUtils;
            std::shared_ptr<TFileManager> _fileManager;
        };
    }
}
