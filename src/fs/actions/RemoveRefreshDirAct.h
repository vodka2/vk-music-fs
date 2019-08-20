#pragma once
#include <fs/common_fs.h>

namespace vk_music_fs {
    namespace fs {
        class RemoveRefreshDirAct{
        public:
            RemoveRefreshDirAct() {}

            template<typename TData, typename TFunc>
            void doAction(const DirPtr &dir, TFunc func){
                auto &curOffsetCnt = std::get<TData>(*dir->getDirExtra());
                if(curOffsetCnt.getRefreshDir() != nullptr) {
                    dir->removeItem(curOffsetCnt.getRefreshDir()->getName());
                }
                curOffsetCnt.setRefreshDir(nullptr);
                func();
            }

        };
    }
}