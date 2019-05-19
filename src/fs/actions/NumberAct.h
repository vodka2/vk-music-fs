#pragma once
#include <fs/common_fs.h>
#include <fs/Dir.h>
#include <fs/IdGenerator.h>

namespace vk_music_fs {
    namespace fs {
        template <typename TFsUtils>
        class NumberAct {
        public:
            NumberAct(
                    const std::shared_ptr<TFsUtils> &utils,
                    const std::shared_ptr<IdGenerator> &idGenerator
            ) : _fsUtils(utils), _idGenerator(idGenerator){
            }

            template<typename TData, typename TFunc>
            void doAction(
                    const DirPtr &dir, const std::string &dirName,
                    bool leaveDirs, const QueryParams &query, TFunc func
            ) {
                TData curOffsetCnt = std::get<TData>(*dir->getDirExtra());
                uint_fast32_t queryOffset = 0, queryCnt = 0;
                bool needMakeQuery = true;
                if(query.type == QueryParams::Type::TWO_NUMBERS){
                    if(leaveDirs){
                        _fsUtils->deleteAllFiles(dir);
                    } else {
                        dir->clear();
                    }
                    curOffsetCnt.setOffset(query.first);
                    curOffsetCnt.setCnt(query.second);

                    queryOffset = query.first;
                    queryCnt = query.second;
                } else {
                    if(curOffsetCnt.getCounterDir() != nullptr) {
                        dir->removeItem(curOffsetCnt.getCounterDir()->getName());
                    }
                    if(curOffsetCnt.getCnt() >= query.first){
                        needMakeQuery = false;
                        _fsUtils->limitFiles(dir, query.first);
                    } else {
                        queryOffset = curOffsetCnt.getOffset() + curOffsetCnt.getCnt();
                        queryCnt = query.first - curOffsetCnt.getCnt();
                    }
                    curOffsetCnt.setCnt(query.first);
                }
                if(needMakeQuery) {
                    func(queryOffset, queryCnt);
                }
                auto cntDir = std::make_shared<Dir>(
                        dirName, _idGenerator->getNextId(), std::nullopt, dir
                );
                dir->addItem(cntDir);
                curOffsetCnt.setCounterDir(cntDir);
                std::get<TData>(*dir->getDirExtra()) = curOffsetCnt;
            }

        private:
            std::shared_ptr<TFsUtils> _fsUtils;
            std::shared_ptr<IdGenerator> _idGenerator;
        };
    }
}
