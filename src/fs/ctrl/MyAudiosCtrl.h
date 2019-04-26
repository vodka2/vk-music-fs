#pragma once

#include <fs/common_fs.h>
#include <fs/FsSettings.h>
#include <fs/FsException.h>
#include <boost/algorithm/string/predicate.hpp>
#include <fs/FsPath.h>
#include <fs/OffsetCnt.h>
#include <fs/Dir.h>
#include <fs/File.h>
#include <fs/IdGenerator.h>
#include <RemoteFile.h>
#include <regex>
#include "ThrowExCtrl.h"

namespace vk_music_fs {
    namespace fs {
        template <typename TFsUtils, typename TFileObtainer>
        class MyAudiosCtrl : public ThrowExCtrl{
        public:
            MyAudiosCtrl(
                    const std::shared_ptr<TFsUtils> &utils,
                    const std::shared_ptr<TFileObtainer> &fileObtainer,
                    const std::shared_ptr<FsSettings> &settings,
                    const std::shared_ptr<IdGenerator> &idGenerator
            ) : _fsUtils(utils), _fileObtainer(fileObtainer), _idGenerator(idGenerator),
                _settings(settings){
            }

            DirPtr getCtrlDir(){
                return _ctrlDir;
            }

            void setRootDir(const DirPtr &dir){
                _ctrlDir = std::make_shared<Dir>(
                        DIR_NAME, _idGenerator->getNextId(), OffsetCnt{0,0,DirPtr{},DirPtr{}}, dir
                );
                dir->addItem(_ctrlDir);
            }

            void checkCreateDirPath(FsPath &path){
                auto parent = path.getAll().front();
                if(parent.getId() != _ctrlDir->getId()){
                    throw FsException("Can't create dir not in the root of My Audios dir");
                }
            }

            void createDir(FsPath &path) {
                checkCreateDirPath(path);
                auto dirName = path.getStringParts().back();
                QueryParams query = _fsUtils->parseQuery(dirName);
                if(query.type == QueryParams::Type::TWO_NUMBERS || query.type == QueryParams::Type::ONE_NUMBER){
                    OffsetCnt curOffsetCnt = std::get<OffsetCnt>(*_ctrlDir->getDirExtra());
                    if(curOffsetCnt.getRefreshDir() != nullptr) {
                        _ctrlDir->removeItem(curOffsetCnt.getRefreshDir()->getName());
                    }
                    uint_fast32_t queryOffset = 0, queryCnt = 0;
                    bool needMakeQuery = true;
                    if(query.type == QueryParams::Type::TWO_NUMBERS){
                        _ctrlDir->clear();
                        curOffsetCnt.setOffset(query.first);
                        curOffsetCnt.setCnt(query.second);
                        queryOffset = query.first;
                        queryCnt = query.second;
                    } else {
                        if(curOffsetCnt.getCounterDir() != nullptr) {
                            _ctrlDir->removeItem(curOffsetCnt.getCounterDir()->getName());
                        }
                        if(curOffsetCnt.getCnt() >= query.first){
                            needMakeQuery = false;
                            _fsUtils->limitFiles(_ctrlDir, query.first);
                        } else {
                            queryOffset = curOffsetCnt.getOffset() + curOffsetCnt.getCnt();
                            queryCnt = query.first - curOffsetCnt.getCnt();
                        }
                        curOffsetCnt.setCnt(query.first);
                    }
                    if(needMakeQuery) {
                        _fsUtils->addFilesToDir(
                                _ctrlDir,
                                _fileObtainer->getMyAudios(queryOffset, queryCnt),
                                _idGenerator,
                                _settings->getMp3Ext()
                        );
                    }
                    auto cntDir = std::make_shared<Dir>(
                            dirName, _idGenerator->getNextId(), std::nullopt, _ctrlDir
                    );
                    _ctrlDir->addItem(cntDir);
                    curOffsetCnt.setCounterDir(cntDir);
                    curOffsetCnt.setRefreshDir(nullptr);
                    std::get<OffsetCnt>(*_ctrlDir->getDirExtra()) = curOffsetCnt;
                } else if(std::regex_match(dirName, std::regex{"^(r|regex)[0-9]*$"})){
                    OffsetCnt curOffsetCnt = std::get<OffsetCnt>(*_ctrlDir->getDirExtra());
                    if(curOffsetCnt.getRefreshDir() != nullptr) {
                        _ctrlDir->removeItem(curOffsetCnt.getRefreshDir()->getName());
                    }
                    _fsUtils->deleteAllFiles(_ctrlDir);
                    auto refreshDir = std::make_shared<Dir>(
                            dirName, _idGenerator->getNextId(), std::nullopt, _ctrlDir
                    );
                    _fsUtils->addFilesToDir(
                            _ctrlDir,
                            _fileObtainer->getMyAudios(curOffsetCnt.getOffset(), curOffsetCnt.getCnt()),
                            _idGenerator,
                            _settings->getMp3Ext()
                    );
                    curOffsetCnt.setRefreshDir(refreshDir);
                    _ctrlDir->addItem(refreshDir);
                    std::get<OffsetCnt>(*_ctrlDir->getDirExtra()) = curOffsetCnt;
                } else {
                    throw FsException("Can't create non-counter, non-refresh dir in My Audios dir");
                }
            }

            std::string getDirName(){
                return DIR_NAME;
            }
        private:
            constexpr const static char *DIR_NAME = "My audios";
            std::shared_ptr<TFsUtils> _fsUtils;
            std::shared_ptr<TFileObtainer> _fileObtainer;
            std::shared_ptr<IdGenerator> _idGenerator;
            std::shared_ptr<FsSettings> _settings;
            DirPtr _ctrlDir;
        };
    }
}
