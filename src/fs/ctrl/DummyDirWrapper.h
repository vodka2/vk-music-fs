#pragma once

#include <fs/common_fs.h>
#include <RemoteFile.h>
#include <fs/FsPath.h>
#include <fs/FsSettings.h>
#include <fs/IdGenerator.h>
#include <fs/FsException.h>
#include <fs/DirOrFile.h>
#include <fs/Dir.h>
#include "RedirectCtrl.h"

namespace vk_music_fs {
    namespace fs {
        template <typename TCtrl, typename TFsUtils>
        class DummyDirWrapper : public RedirectCtrl<TCtrl> {
        public:
            DummyDirWrapper(
                    const std::shared_ptr<TFsUtils> &utils,
                    const std::shared_ptr<TCtrl> &ctrl,
                    const std::shared_ptr<FsSettings> &fsSettings,
                    const std::shared_ptr<IdGenerator> &idGenerator
            ): RedirectCtrl<TCtrl>(ctrl), _fsUtils(utils), _ctrl(ctrl), _fsSettings(fsSettings), _idGenerator(idGenerator){}

            void createDir(const std::string &dirPath) {
                FsPath path = _fsUtils->findPath(_ctrl->getCtrlDir(), _ctrl->transformPath(dirPath), FsPath::WITH_PARENT_DIR);
                FsPathUnlocker unlocker{path};
                if(!path.isParentPathMatched() || !path.isParentPathDir()){
                    throw FsException("Can't create dir " + dirPath + " because parent dir does not exist");
                }
                auto parent = path.getAll().front();
                if(_fsSettings->isCreateDummyDirs()) {
                    _ctrl->checkCreateDirPath(path);
                    auto dummyDir = std::make_shared<Dir>(
                            path.getStringParts().back(),
                            _idGenerator->getNextId(),
                            DummyDirMarker{},
                            _ctrl->getCtrlDir()
                    );
                    parent.dir()->addItem(dummyDir);
                } else {
                    _ctrl->createDir(path);
                }
            }

            void rename(const std::string &oldPath, const std::string &newPath){
                FsPath oldFsPath = _fsUtils->findPath(_ctrl->getCtrlDir(), _ctrl->transformPath(oldPath), FsPath::WITH_PARENT_DIR);
                FsPath newFsPath = _fsUtils->findPath(_ctrl->getCtrlDir(), _ctrl->transformPath(newPath), {oldFsPath}, FsPath::WITH_PARENT_DIR);
                FsPathUnlocker oldUnlocker{{oldFsPath, newFsPath}};
                if(!oldFsPath.isPathMatched()){
                    throw FsException("Can't rename " + oldPath + " to " + newPath + " because source does not exist");
                }
                if(!newFsPath.isParentPathMatched() || !newFsPath.isParentPathDir()){
                    throw FsException("Can't rename " + oldPath + " to " + newPath + " because parent of target dir does not exist");
                }
                if(_fsSettings->isCreateDummyDirs() && oldFsPath.isPathDir() && std::holds_alternative<DummyDirMarker>(
                        *oldFsPath.getAll().back().dir()->getDirExtra()
                )) {
                    auto parent = oldFsPath.getAll().front().dir();
                    if(parent->getId() != newFsPath.getAll().front().getId()){
                        throw FsException("Dummy dirs " + oldPath + " and " + newPath + " must be in the same dir");
                    }
                    parent->removeItem(oldFsPath.getAll().back().getName());
                    _ctrl->createDir(newFsPath);
                } else {
                    _ctrl->rename(oldFsPath, newFsPath);
                }
            }
        private:
            std::shared_ptr<TFsUtils> _fsUtils;
            std::shared_ptr<IdGenerator> _idGenerator;
            std::shared_ptr<TCtrl> _ctrl;
            std::shared_ptr<FsSettings> _fsSettings;
        };
    }
}
