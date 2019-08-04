#pragma once

#include <fs/FsSettings.h>
#include <fs/IdGenerator.h>
#include <fs/OffsetCnt.h>
#include <fs/OffsetCntName.h>
#include <fs/Dir.h>
#include <fs/File.h>
#include <boost/filesystem/convenience.hpp>
#include "ThrowExCtrl.h"
#include <regex>
#include <fs/actions/act.h>

namespace vk_music_fs {
    namespace fs {
        template <typename TFsUtils, typename TFileObtainer, typename THelper>
        class SearchSongNameCtrl : public ThrowExCtrl {
        public:
            SearchSongNameCtrl(
                    const std::shared_ptr<TFsUtils> &utils,
                    const std::shared_ptr<TFileObtainer> &fileObtainer,
                    const std::shared_ptr<FsSettings> &settings,
                    const std::shared_ptr<IdGenerator> &idGenerator,
                    const std::shared_ptr<THelper> &helper,
                    const ActTuple<TFsUtils> &acts
            ) : _fsUtils(utils), _fileObtainer(fileObtainer), _idGenerator(idGenerator),
                _settings(settings), _acts(acts), _helper(helper){
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
                            _helper->searchFiles(_fileObtainer, searchName, 0, _settings->getNumSearchFiles()),
                            _idGenerator,
                            _settings->getMp3Ext()
                    );
                } else {
                    getAct<NumberAct>(_acts)->template doAction<OffsetCntName>(parent, dirName, true, query,
                            [this, parent] (uint_fast32_t offset, uint_fast32_t cnt) {
                                _fsUtils->addFilesToDir(
                                        parent,
                                        _helper->searchFiles(
                                                _fileObtainer,
                                                std::get<OffsetCntName>(*parent->getDirExtra()).getName(),
                                                offset,
                                                cnt
                                        ),
                                        _idGenerator,
                                        _settings->getMp3Ext()
                                );
                            }
                    );

                }
            }

            void rename(FsPath& oldPath, FsPath &newPath) {
                if(!oldPath.getAll().back().isFile()){
                    throw FsException("Can't rename non-file " + oldPath.getAll().back().getName() + " to " + newPath.getAll().back().getName());
                }
                auto origFname = boost::filesystem::change_extension(oldPath.getStringParts().back(), "").string();
                if(newPath.getStringParts().back() !=  origFname + "_a" + _settings->getMp3Ext()){
                    throw FsException(
                            "Currently only adding _a suffix is supported, not " +
                            oldPath.getAll().back().getName() + " to " + newPath.getAll().back().getName()
                    );
                }
                auto dir = oldPath.getAll().front().dir();
                auto prevFile = dir->getItem(oldPath.getStringParts().back()).file();
                auto remFile = std::get<RemoteFile>(*prevFile->getExtra());
                _fileObtainer->addToMyAudios(remFile.getOwnerId(), remFile.getFileId());
                dir->removeItem(oldPath.getStringParts().back());
                dir->addItem(std::make_shared<File>(
                        newPath.getStringParts().back(),
                        _idGenerator->getNextId(),
                        prevFile->getTime(),
                        remFile,
                        dir
                ));
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

            void deleteDir(const std::string &path) {
                FsPath fsPath = _fsUtils->findPath(_ctrlDir, _fsUtils->stripPathPrefix(path, getDirName()), FsPath::WITH_PARENT_DIR);
                FsPathUnlocker unlocker{fsPath};
                if(!fsPath.isPathMatched() || !fsPath.isPathDir() || fsPath.getAll().back().getId() == _ctrlDir->getId()){
                    throw FsException("Directory does not exist " + path);
                }
                fsPath.getAll().front().dir()->removeItem(fsPath.getAll().back().getName());
            }

            void deleteFile(const std::string &path) {
                FsPath fsPath = _fsUtils->findPath(_ctrlDir, _fsUtils->stripPathPrefix(path, getDirName()), FsPath::WITH_PARENT_DIR);
                FsPathUnlocker unlocker{fsPath};
                if(!fsPath.isPathMatched() || !fsPath.getAll().back().isFile()){
                    throw FsException("File does not exist " + path);
                }
                fsPath.getAll().front().dir()->removeItem(fsPath.getAll().back().getName());
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
            DirPtr _ctrlDir;
            ActTuple<TFsUtils> _acts;
        };
    }
}