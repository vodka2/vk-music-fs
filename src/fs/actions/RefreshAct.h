#pragma once
#include <fs/common_fs.h>
#include <fs/Dir.h>
#include <fs/OffsetCnt.h>
#include <fs/IdGenerator.h>

namespace vk_music_fs {
    namespace fs {
        template <typename TFsUtils>
        class RefreshAct {
        public:
            RefreshAct(
                    const std::shared_ptr<TFsUtils> &utils,
                    const std::shared_ptr<IdGenerator> &idGenerator
            ) : _fsUtils(utils), _idGenerator(idGenerator){
            }

            template<typename TData, typename TFunc>
            void doAction(const DirPtr &dir, const std::string &dirName, TFunc func) {
                doAction<TData>(dir, dirName, true, func);
            }

            template<typename TData, typename TFunc>
            void doAction(const DirPtr &dir, const std::string &dirName, bool leaveDirs, TFunc func) {
                TData curOffsetCnt = std::get<TData>(*dir->getDirExtra());
                if (curOffsetCnt.getRefreshDir() != nullptr) {
                    dir->removeItem(curOffsetCnt.getRefreshDir()->getName());
                }
                if (leaveDirs) {
                    _fsUtils->deleteAllFiles(dir);
                } else {
                    dir->clear();
                }
                auto refreshDir = std::make_shared<Dir>(
                        dirName, _idGenerator->getNextId(), std::nullopt, dir
                );
                func(curOffsetCnt.getOffset(), curOffsetCnt.getCnt());
                curOffsetCnt.setRefreshDir(refreshDir);
                dir->addItem(refreshDir);
                std::get<TData>(*dir->getDirExtra()) = curOffsetCnt;
            }

        private:
            std::shared_ptr<TFsUtils> _fsUtils;
            std::shared_ptr<IdGenerator> _idGenerator;
        };
    }
}