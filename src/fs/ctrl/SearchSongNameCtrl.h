#pragma once

#include <fs/FsSettings.h>
#include <fs/IdGenerator.h>
#include <fs/OffsetCnt.h>
#include <fs/OffsetCntName.h>
#include <fs/Dir.h>
#include <fs/File.h>
#include "ThrowExCtrl.h"

namespace vk_music_fs {
    namespace fs {
        template <typename TFsUtils, typename TFileObtainer>
        class SearchSongNameCtrl : public ThrowExCtrl {
        public:
            SearchSongNameCtrl(
                    const std::shared_ptr<TFsUtils> &utils,
                    const std::shared_ptr<TFileObtainer> &fileObtainer,
                    const std::shared_ptr<FsSettings> &settings,
                    const std::shared_ptr<IdGenerator> &idGenerator
            ) : _fsUtils(utils), _fileObtainer(fileObtainer), _idGenerator(idGenerator),
                _settings(settings){
            }

            void checkCreateDirPath(FsPath &path){
                auto parent = path.getAll().front();
                if(
                        parent.getId() != _ctrlDir->getId() &&
                        (!parent.dir()->getDirExtra() || !std::holds_alternative<OffsetCntName>(*parent.dir()->getDirExtra()))
                ){
                    throw FsException("Can't create dir inside counter dir");
                }
            }

            void createDir(FsPath &path) {
                checkCreateDirPath(path);
                auto dirName = path.getStringParts().back();
                auto parent = path.getAll().back().dir();
                QueryParams query = _fsUtils->parseQuery(dirName);
                if(query.type == QueryParams::Type::STRING || !parent->getDirExtra()){
                    auto searchName = parent->getDirExtra() ?
                            (std::get<OffsetCntName>(*parent->getDirExtra()).getName() + " " + dirName) :
                            dirName
                    ;
                    auto offsetCntName = OffsetCntName{0, _settings->getNumSearchFiles(), searchName, nullptr};
                    auto searchDir = std::make_shared<Dir>(
                            dirName, _idGenerator->getNextId(), offsetCntName, parent
                    );
                    parent->addItem(searchDir);
                    _fsUtils->addFilesToDir(
                            searchDir,
                            _fileObtainer->searchBySongName(searchName, 0, _settings->getNumSearchFiles()),
                            _idGenerator,
                            _settings->getMp3Ext()
                    );
                } else {
                    auto curOffsetCntName = std::get<OffsetCntName>(*parent->getDirExtra());
                    uint_fast32_t queryOffset = 0, queryCnt = 0;
                    bool needMakeQuery = true;
                    if(query.type == QueryParams::Type::TWO_NUMBERS){
                        _fsUtils->deleteAllFiles(parent);
                        curOffsetCntName.setOffset(query.first);
                        curOffsetCntName.setCnt(query.second);

                        queryOffset = query.first;
                        queryCnt = query.second;
                    } else {
                        if(curOffsetCntName.getCounterDir() != nullptr) {
                            parent->removeItem(curOffsetCntName.getCounterDir()->getName());
                        }
                        if(curOffsetCntName.getCnt() >= query.first){
                            needMakeQuery = false;
                            _fsUtils->limitFiles(parent, query.first);
                        } else {
                            queryOffset = curOffsetCntName.getOffset() + curOffsetCntName.getCnt();
                            queryCnt = query.first - curOffsetCntName.getCnt();
                        }
                        curOffsetCntName.setCnt(query.first);
                    }
                    if(needMakeQuery) {
                        _fsUtils->addFilesToDir(
                                parent,
                                _fileObtainer->searchBySongName(curOffsetCntName.getName(), queryOffset, queryCnt),
                                _idGenerator,
                                _settings->getMp3Ext()
                        );
                    }
                    auto cntDir = std::make_shared<Dir>(
                            dirName, _idGenerator->getNextId(), std::nullopt, parent
                    );
                    parent->addItem(cntDir);
                    curOffsetCntName.setCounterDir(cntDir);
                    std::get<OffsetCntName>(*parent->getDirExtra()) = curOffsetCntName;
                }
            }

            DirPtr getCtrlDir(){
                return _ctrlDir;
            }

            void setRootDir(const DirPtr &dir){
                _ctrlDir = std::make_shared<Dir>(
                        DIR_NAME, _idGenerator->getNextId(), std::nullopt, dir
                );
                dir->addItem(_ctrlDir);
            }

            std::string getDirName(){
                return DIR_NAME;
            }
        private:
            constexpr const static char *DIR_NAME = "Search";
            std::shared_ptr<TFsUtils> _fsUtils;
            std::shared_ptr<TFileObtainer> _fileObtainer;
            std::shared_ptr<IdGenerator> _idGenerator;
            std::shared_ptr<FsSettings> _settings;
            DirPtr _ctrlDir;
        };
    }
}