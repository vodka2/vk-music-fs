#pragma once

#include "RedirectCtrl.h"
#include <fs/DirOrFile.h>
#include <fs/Dir.h>
#include <fs/File.h>
#include <fs/OffsetCntRemoteFile.h>
#include <fs/FsException.h>
#include <boost/filesystem.hpp>
#include <fs/FsSettings.h>
#include <fs/IdGenerator.h>
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
                if (
                        !parent.isDir() ||
                        !parent.dir()->getDirExtra() ||
                        !std::holds_alternative<OffsetCntRemoteFile>(*parent.dir()->getDirExtra())
                ) {
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

            void rename(FsPath& oldPath, FsPath &newPath) {
                if(!oldPath.getAll().back().isFile()){
                    _ctrl->rename(oldPath, newPath);
                    return;
                }
                auto origFname = boost::filesystem::change_extension(oldPath.getStringParts().back(), "").string();
                if(newPath.getStringParts().back() != origFname + "_s" + _settings->getMp3Ext()){
                    _ctrl->rename(oldPath, newPath);
                    return;
                }
                auto dir = oldPath.getAll().front().dir();
                auto prevFile = dir->getItem(oldPath.getStringParts().back()).file();
                auto remFile = std::get<RemoteFile>(*prevFile->getExtra());
                dir->removeItem(oldPath.getStringParts().back());
                dir->addItem(std::make_shared<File>(
                        newPath.getStringParts().back(),
                        _idGenerator->getNextId(),
                        prevFile->getTime(),
                        remFile,
                        dir
                ));
                auto similarDir = std::make_shared<Dir>(
                        origFname, _idGenerator->getNextId(),
                        OffsetCntRemoteFile{0, _settings->getNumSearchFiles(), DirWPtr{}, RemoteFileId{remFile}},
                        dir
                );
                dir->addItem(similarDir);
                _asyncFsManager->createFiles(
                        similarDir,
                        _fileObtainer->searchSimilar(RemoteFileId{remFile}, 0, _settings->getNumSearchFiles())
                );
            }

        private:
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

