#pragma once
#include <fs/common_fs.h>
#include <fs/Dir.h>
#include <fs/IdGenerator.h>
#include <fs/FsPath.h>
#include <fs/FsException.h>

namespace vk_music_fs {
    namespace fs {
        template <typename TFsUtils>
        class DeleteDirAct {
        public:
            DeleteDirAct(
                    const std::shared_ptr<TFsUtils> &utils
            ) : _fsUtils(utils) {
            }

            template <typename TCheckFunc>
            void doAction(FsPath &fsPath, TCheckFunc func) {
                if(!fsPath.isPathMatched() || !fsPath.isPathDir() || func()){
                    throw FsException("Directory does not exist " + fsPath.getStringParts().back());
                }
                if (func()) {
                    throw FsException("Can't delete dir " + fsPath.getStringParts().back());
                }
                fsPath.getAll().front().dir()->removeItem(fsPath.getAll().back().getName());
            }

        private:
            std::shared_ptr<TFsUtils> _fsUtils;
        };
    }
}
