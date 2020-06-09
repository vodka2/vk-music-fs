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

            std::vector<FileInfo> getFileInfos(const std::string &dirname) {
                FsPath fsPath = _fsUtils->findPath(_ctrl->getCtrlDir(), _ctrl->transformPath(dirname));
                FsPathUnlocker unlocker{fsPath};
                if (!fsPath.isPathMatched() || !fsPath.getLast().isDir()) {
                    throw FsException("Dir " + dirname + " does not exist");
                }
                DirPtr dir = fsPath.getLast().dir();
                std::vector<FileInfo> retVector;
                for(auto &item: dir->getContents()) {
                    if (item.second.isFile()) {
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
                auto remoteFile = _fsUtils->getRemoteFile(fsPath, filename);
                return _fileManager->open(remoteFile, filename);
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
