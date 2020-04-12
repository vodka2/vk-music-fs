#pragma once
#include <fs/common_fs.h>
#include <fs/Dir.h>
#include <fs/IdGenerator.h>
#include <fs/FsPath.h>
#include <fs/FsException.h>

namespace vk_music_fs {
    namespace fs {
        template <typename TFsUtils>
        class DeleteFileAct {
        public:
            DeleteFileAct(
                    const std::shared_ptr<TFsUtils> &utils
            ) : _fsUtils(utils) {
            }

            void doAction(FsPath &fsPath) {
                if(!fsPath.isPathMatched() || !fsPath.getAll().back().isFile()){
                    throw FsException("File does not exist " + fsPath.getStringParts().back());
                }
                fsPath.getAll().front().dir()->removeItem(fsPath.getAll().back().getName());
            }

        private:
            std::shared_ptr<TFsUtils> _fsUtils;
        };
    }
}
