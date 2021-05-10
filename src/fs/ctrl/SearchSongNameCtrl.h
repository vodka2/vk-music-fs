#pragma once

#include <fs/FsSettings.h>
#include <common/IdGenerator.h>
#include <fs/OffsetCnt.h>
#include <fs/OffsetCntName.h>
#include <fs/Dir.h>
#include <fs/File.h>
#include <boost/filesystem/convenience.hpp>
#include "ThrowExCtrl.h"
#include <regex>
#include <fs/actions/act.h>
#include <fs/actions/DeleteDirAct.h>
#include <fs/actions/DeleteFileAct.h>

namespace vk_music_fs {
    namespace fs {
        template <typename TFsUtils, typename TFileObtainer, typename THelper, typename TAsyncFsManager>
        class SearchSongNameCtrl : public ThrowExCtrl {
        public:
            SearchSongNameCtrl(
                    const std::shared_ptr<TFsUtils> &utils,
                    const std::shared_ptr<TFileObtainer> &fileObtainer,
                    const std::shared_ptr<FsSettings> &settings,
                    const std::shared_ptr<IdGenerator> &idGenerator,
                    const std::shared_ptr<THelper> &helper,
                    const std::shared_ptr<TAsyncFsManager> &asyncFsManager,
                    const ActTuple<TFsUtils> &acts
            ) : _fsUtils(utils), _fileObtainer(fileObtainer), _idGenerator(idGenerator),
                _settings(settings), _acts(acts), _helper(helper), _asyncFsManager(asyncFsManager){
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
                    _asyncFsManager->createFiles(
                            searchDir,
                            _helper->searchFiles(_fileObtainer, searchName, 0, _settings->getNumSearchFiles())
                    );
                } else {
                    getAct<NumberAct>(_acts)->template doAction<OffsetCntName>(
                            parent, dirName, query,
                            _fsUtils->template getCounterDirLeaver<OffsetCntName>(),
                            [this, parent] (uint_fast32_t offset, uint_fast32_t cnt) {
                                _asyncFsManager->createFiles(
                                        parent,
                                        _helper->searchFiles(
                                                _fileObtainer,
                                                std::get<OffsetCntName>(*parent->getDirExtra()).getName(),
                                                offset,
                                                cnt
                                        )
                                );
                            }
                    );

                }
            }

            DirPtr getCtrlDir(){
                return _ctrlDir;
            }

            void setRootDir(const DirPtr &dir){
                _ctrlDir = std::make_shared<Dir>(
                        getDirName(), _idGenerator->getNextId(), std::nullopt, dir
                );
                dir->addItem(_ctrlDir);
            }

            void deleteDir(FsPath &fsPath) {
                getAct<DeleteDirAct>(_acts)->doAction(fsPath, [&fsPath, this] () {
                    return fsPath.getAll().back().getId() == _ctrlDir->getId();
                });
            }

            void deleteFile(FsPath &fsPath) {
                getAct<DeleteFileAct>(_acts)->doAction(fsPath);
            }

            std::string getDirName(){
                return _helper->getDirName();
            }
        private:
            std::shared_ptr<TFsUtils> _fsUtils;
            std::shared_ptr<TFileObtainer> _fileObtainer;
            std::shared_ptr<THelper> _helper;
            std::shared_ptr<IdGenerator> _idGenerator;
            std::shared_ptr<FsSettings> _settings;
            std::shared_ptr<TAsyncFsManager> _asyncFsManager;
            DirPtr _ctrlDir;
            ActTuple<TFsUtils> _acts;
        };
    }
}