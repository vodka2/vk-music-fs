#pragma once

#include <fs/common_fs.h>
#include <mp3core/RemoteFile.h>
#include <fs/FsException.h>
#include "ThrowExCtrl.h"

namespace vk_music_fs {
    namespace fs {
        template <typename TFsUtils>
        class RootCtrl : public ThrowExCtrl{
        public:
            RootCtrl(const std::shared_ptr<TFsUtils> &utils) : _fsUtils(utils){}

            void setRootDir(const DirPtr &dir){
                _ctrlDir = dir;
            }

            bool supports(const std::string &path) {
                return true;
            }

            std::vector<std::string> getEntries(const std::string &path) {
                return _fsUtils->getEntries(_ctrlDir, path, path);
            }

            FileOrDirMeta getMeta(const std::string &path) {
                return _fsUtils->getMeta(_ctrlDir, path);
            }
        private:
            constexpr const static char *DIR_NAME = "/";
            std::shared_ptr<TFsUtils> _fsUtils;
            DirPtr _ctrlDir;

        };
    }
}