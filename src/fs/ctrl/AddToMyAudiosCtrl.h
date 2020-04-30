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
        template <typename TCtrl, typename TFsUtils, typename TFileObtainer>
        class AddToMyAudiosCtrl: public RedirectCtrl<TCtrl> {
        public:
            AddToMyAudiosCtrl(
                const std::shared_ptr<TCtrl> &ctrl, const std::shared_ptr<TFileObtainer> &fileObtainer,
                const std::shared_ptr<FsSettings> &fsSettings, const std::shared_ptr<IdGenerator> &idGenerator,
                const ActTuple<TFsUtils> &acts
            )
            : RedirectCtrl<TCtrl>(ctrl), _ctrl(ctrl), _settings(fsSettings), _idGenerator(idGenerator),
              _fileObtainer(fileObtainer), _acts(acts) {}

            void rename(FsPath& oldPath, FsPath &newPath) {
                if(!oldPath.getAll().back().isFile()){
                    _ctrl->rename(oldPath, newPath);
                    return;
                }
                auto origFname = boost::filesystem::change_extension(oldPath.getStringParts().back(), "").string();
                if(newPath.getStringParts().back() != origFname + "_a" + _settings->getMp3Ext()){
                    _ctrl->rename(oldPath, newPath);
                    return;
                }
                auto dir = oldPath.getAll().front().dir();
                auto prevFile = dir->getItem(oldPath.getStringParts().back()).file();
                auto remFile = std::get<RemoteFile>(*prevFile->getExtra());
                _fileObtainer->addToMyAudios(remFile.getOwnerId(), remFile.getFileId());
                dir->removeItem(oldPath.getStringParts().back());
                dir->addItem(std::make_shared<File>(
                        newPath.getStringParts().back(),
                        _idGenerator->getNextId(),
                        prevFile->getTime(),
                        remFile,
                        dir
                ));
            }

        private:
            std::shared_ptr<TCtrl> _ctrl;
            std::shared_ptr<FsSettings> _settings;
            std::shared_ptr<IdGenerator> _idGenerator;
            std::shared_ptr<TFileObtainer> _fileObtainer;
            ActTuple<TFsUtils> _acts;
        };
    }
}

