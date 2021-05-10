#pragma once

#include "RedirectCtrl.h"
#include <fs/DirOrFile.h>
#include <fs/Dir.h>
#include <fs/File.h>
#include <fs/OffsetCntRemoteFile.h>
#include <fs/FsException.h>
#include <boost/filesystem.hpp>
#include <fs/FsSettings.h>
#include <common/IdGenerator.h>
#include <fs/actions/NumberAct.h>
#include <fs/actions/act.h>

namespace vk_music_fs {
    namespace fs {
        template <typename TCtrl, typename TFsUtils, typename TFileObtainer, typename TAsyncFsManager>
        class SimilarCtrl: public RedirectCtrl<TCtrl> {
        public:
            SimilarCtrl(
                const std::shared_ptr<TCtrl> &ctrl, const std::shared_ptr<TFsUtils> &fsUtils,
                const std::shared_ptr<FsSettings> &fsSettings, const std::shared_ptr<IdGenerator> &idGenerator,
                const std::shared_ptr<TAsyncFsManager> &asyncManager, const std::shared_ptr<TFileObtainer> &fileObtainer,
                const ActTuple<TFsUtils> &acts
            )
            : RedirectCtrl<TCtrl>(ctrl), _ctrl(ctrl), _fsUtils(fsUtils), _settings(fsSettings), _idGenerator(idGenerator),
              _asyncFsManager(asyncManager), _fileObtainer(fileObtainer), _acts(acts) {}

            void createDir(FsPath &path) {
                auto parent = path.getAll().front();
                if (!isSimilarDir(parent)) {
                    _ctrl->createDir(path);
                    return;
                }
                auto dirName = path.getStringParts().back();
                auto parentDir = parent.dir();
                QueryParams query = _fsUtils->parseQuery(dirName);
                if (query.type != QueryParams::Type::STRING) {
                    getAct<NumberAct>(_acts)->template doAction<OffsetCntRemoteFile>(
                            parentDir, dirName, query,
                            _fsUtils->template getCounterDirLeaver<OffsetCntRemoteFile>(),
                            [this, parentDir] (uint_fast32_t offset, uint_fast32_t cnt) {
                                _asyncFsManager->createFiles(
                                        parentDir,
                                        _fileObtainer->searchSimilar(
                                                std::get<OffsetCntRemoteFile>(*parentDir->getDirExtra()).getRemoteFileId(),
                                                offset, cnt
                                        )
                                );
                            }
                    );
                } else {
                    throw FsException("Can't create dir " + dirName);
                }
            }

            void deleteDir(const std::string &path) {
                FsPath fsPath = _fsUtils->findPath(
                        _ctrl->getCtrlDir(), _fsUtils->stripPathPrefix(path, _ctrl->getDirName()),
                        FsPath::WITH_PARENT_DIR
                );
                FsPathUnlocker unlocker{fsPath};
                if (!isSimilarDir(fsPath.getAll().back()) && !isSimilarDir(fsPath.getAll().front())) {
                    _ctrl->deleteDir(fsPath);
                }
                getAct<DeleteDirAct>(_acts)->doAction(fsPath, [&fsPath, this] () {return false;});
            }

            void deleteFile(const std::string &path) {
                FsPath fsPath = _fsUtils->findPath(
                        _ctrl->getCtrlDir(), _fsUtils->stripPathPrefix(path, _ctrl->getDirName()),
                        FsPath::WITH_PARENT_DIR
                );
                FsPathUnlocker unlocker{fsPath};
                if (!isSimilarDir(fsPath.getAll().front())) {
                    _ctrl->deleteFile(fsPath);
                }
                getAct<DeleteFileAct>(_acts)->doAction(fsPath);
            }

            void rename(FsPath& oldPath, FsPath &newPath) {
                if(!oldPath.getAll().back().isFile() ||
                    !std::holds_alternative<RemoteFile>(*oldPath.getAll().back().file()->getExtra())) {
                    _ctrl->rename(oldPath, newPath);
                    return;
                }
                auto origFname = boost::filesystem::change_extension(oldPath.getStringParts().back(), "").string();
                auto newFname = boost::filesystem::change_extension(newPath.getStringParts().back(), "").string();
                if (newFname + "_s" == origFname) {
                    auto dir = oldPath.getAll().front().dir();
                    auto newFile = dir->renameFile(oldPath.getStringParts().back(), newPath.getStringParts().back(),
                            _idGenerator->getNextId());
                } else if(newFname == origFname + "_s") {
                    auto dir = oldPath.getAll().front().dir();
                    if (dir->hasItem(origFname)) {
                        throw FsException("Directory with name " + origFname + " already exists");
                    }
                    auto remoteFile = std::get<RemoteFile>(*dir->getItem(oldPath.getStringParts().back()).file()->getExtra());
                    auto newFiles =
                            _fileObtainer->searchSimilar(RemoteFileId{remoteFile}, 0, _settings->getNumSearchFiles());
                    dir->renameFile(oldPath.getStringParts().back(), newPath.getStringParts().back(),
                            _idGenerator->getNextId());
                    auto similarDir = std::make_shared<Dir>(
                            origFname, _idGenerator->getNextId(),
                            OffsetCntRemoteFile{0, _settings->getNumSearchFiles(), DirWPtr{},
                                                RemoteFileId{remoteFile}},
                            dir
                    );
                    dir->addItem(similarDir);
                    _asyncFsManager->createFiles(similarDir, newFiles);
                } else {
                    _ctrl->rename(oldPath, newPath);
                }
            }

        private:
            bool isSimilarDir(DirOrFile dirOrFile) {
                return (
                        dirOrFile.isDir() &&
                        dirOrFile.dir()->getDirExtra() &&
                        std::holds_alternative<OffsetCntRemoteFile>(*dirOrFile.dir()->getDirExtra())
                );
            }

            std::shared_ptr<TCtrl> _ctrl;
            std::shared_ptr<TFsUtils> _fsUtils;
            std::shared_ptr<TAsyncFsManager> _asyncFsManager;
            std::shared_ptr<FsSettings> _settings;
            std::shared_ptr<IdGenerator> _idGenerator;
            std::shared_ptr<TFileObtainer> _fileObtainer;
            ActTuple<TFsUtils> _acts;
        };
    }
}

