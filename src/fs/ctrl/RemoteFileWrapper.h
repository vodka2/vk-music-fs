#pragma once

#include <fs/common_fs.h>
#include <fs/FsPath.h>
#include <fs/DirOrFile.h>
#include <fs/Dir.h>
#include <fs/File.h>
#include <fs/FsException.h>
#include <main/NoReportException.h>
#include <fs/FileInfo.h>
#include "RedirectCtrl.h"

namespace vk_music_fs {
    namespace fs {
        template <typename TCtrl, typename TFsUtils, typename TFileManager, typename TPhotoManager>
        class RemoteFileWrapper: public RedirectCtrl<TCtrl> {
        public:
            RemoteFileWrapper(
                    std::shared_ptr<TCtrl> ctrl,
                    std::shared_ptr<TFsUtils> fsUtils,
                    std::shared_ptr<TFileManager> fileManager,
                    std::shared_ptr<TPhotoManager> photoManager
            ) : RedirectCtrl<TCtrl>(ctrl), _ctrl(ctrl), _fsUtils(fsUtils),
            _fileManager(fileManager), _photoManager(photoManager){
            }

            std::string transformPath(const std::string &path){
                return _ctrl->transformPath(path);
            }

            std::vector<FileInfo> getFileInfos(const std::string &dirname) {
                FsPath fsPath = _fsUtils->findPath(_ctrl->getCtrlDir(), _ctrl->transformPath(dirname));
                FsPathUnlocker unlocker{fsPath};
                if (!fsPath.isPathMatched() || !fsPath.getLast().isDir()) {
                    throw FsException("Dir " + dirname + " does not exist");
                }
                DirPtr dir = fsPath.getLast().dir();
                std::vector<FileInfo> retVector;
                for(auto &item: dir->getContents()) {
                    if (item.second.isFile() && std::holds_alternative<RemoteFile>(*item.second.file()->getExtra())) {
                        auto extra = std::get<RemoteFile>(*item.second.file()->getExtra());
                        retVector.emplace_back(extra.getArtist(), extra.getTitle(),
                                dirname + "/" + item.second.getName(), item.second.getTime());
                    }
                }
                sort(retVector.begin(), retVector.end(), [](const auto &a, const auto &b) {
                    return a.getTime() < b.getTime();
                });
                return retVector;
            }

            int_fast32_t open(const std::string &filename){
                FsPath fsPath = _fsUtils->findPath(_ctrl->getCtrlDir(), _ctrl->transformPath(filename));
                FsPathUnlocker unlocker{fsPath};
                if (!fsPath.isPathMatched() || fsPath.getLast().isDir() || fsPath.getLast().file()->isHidden()) {
                    throw FsException("File " + filename + " does not exist");
                }
                auto extra = _fsUtils->getFileExtra(fsPath, filename);
                if (extra && std::holds_alternative<RemoteFile>(*extra)) {
                    auto remoteFile = std::get<RemoteFile>(*extra);
                    return _fileManager->open(remoteFile, filename);
                } else if (extra && std::holds_alternative<RemotePhotoFile>(*extra)) {
                    auto remotePhotoFile = std::get<RemotePhotoFile>(*extra);
                    return _photoManager->open(remotePhotoFile, filename);
                } else {
                    throw FsException("Bad file " + filename);
                }
            }

            void createFile(const std::string &filename) {
                FsPath fsPath = _fsUtils->findPath(_ctrl->getCtrlDir(), _ctrl->transformPath(filename), FsPath::WITH_PARENT_DIR);
                FsPathUnlocker unlocker{fsPath};
                if (!fsPath.isPathMatched() || fsPath.getLast().isDir() || !fsPath.getLast().file()->isHidden()) {
                    throw FsException("File " + filename + " does not exist");
                }
                FilePtr file = fsPath.getParent().dir()->getItem(fsPath.getStringParts().back()).file();
                file->setHidden(false);
            }

            uint_fast32_t getFileSize(const std::string &filename){
                FsPath fsPath = _fsUtils->findPath(_ctrl->getCtrlDir(), _ctrl->transformPath(filename));
                FsPathUnlocker unlocker{fsPath};
                if (fsPath.getLast().file()->isHidden()) {
                    throw NoReportException();
                }
                if (!fsPath.isPathMatched() || fsPath.getLast().isDir()) {
                    throw FsException("File " + filename + " does not exist");
                }
                auto extra = _fsUtils->getFileExtra(fsPath, filename);
                if (extra && std::holds_alternative<RemoteFile>(*extra)) {
                    auto remoteFile = std::get<RemoteFile>(*extra);
                    return _fileManager->getFileSize(remoteFile, filename);
                } else if (extra && std::holds_alternative<RemotePhotoFile>(*extra)) {
                    auto remotePhotoFile = std::get<RemotePhotoFile>(*extra);
                    return _photoManager->getFileSize(remotePhotoFile, filename);
                } else {
                    throw FsException("Bad file " + filename);
                }
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
            std::shared_ptr<TPhotoManager> _photoManager;
        };
    }
}
