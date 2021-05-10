#pragma once
#include <fs/common_fs.h>
#include <fs/Dir.h>
#include <common/IdGenerator.h>

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


            template<typename TData, typename TAddFunc, typename TDelFunc>
            void doAction(
                    const DirPtr &dir, const std::string &dirName,
                    const QueryParams &query, TDelFunc delFunc, TAddFunc addFunc
            ) {
                TData curOffsetCnt = std::get<TData>(*dir->getDirExtra());
                uint_fast32_t queryOffset = 0, queryCnt = 0;
                bool needMakeQuery = true;
                if(query.type == QueryParams::Type::TWO_NUMBERS){
                    _fsUtils->deleteItems(dir, delFunc);
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
                        _fsUtils->limitItems(dir, query.first, delFunc);
                    } else {
                        queryOffset = curOffsetCnt.getOffset() + curOffsetCnt.getCnt();
                        queryCnt = query.first - curOffsetCnt.getCnt();
                    }
                    curOffsetCnt.setCnt(query.first);
                }
                if(needMakeQuery) {
                    addFunc(queryOffset, queryCnt);
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
