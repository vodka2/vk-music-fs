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
#include <mp3core/RemoteFile.h>
#include <regex>
#include <fs/actions/act.h>
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
                    const std::shared_ptr<IdGenerator> &idGenerator,
                    const ActTuple<TFsUtils> &acts
            ) : _fsUtils(utils), _fileObtainer(fileObtainer), _idGenerator(idGenerator),
                _settings(settings), _acts(acts){
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
                    getAct<NumberAct>(_acts)->template doAction<OffsetCnt>(_ctrlDir, dirName, false, query,
                        [this] (uint_fast32_t offset, uint_fast32_t cnt) {
                            getAct<RemoveRefreshDirAct>(_acts)->template doAction<OffsetCnt>(
                                    _ctrlDir,
                                    [this, offset, cnt] {
                                        _fsUtils->addFilesToDir(
                                                _ctrlDir,
                                                _fileObtainer->getMyAudios(offset, cnt),
                                                _idGenerator,
                                                _settings->getMp3Ext()
                                        );
                                    }
                            );
                        }
                    );
                } else if(std::regex_match(dirName, std::regex{"^(r|refresh)[0-9]*$"})){
                    getAct<RefreshAct>(_acts)->template doAction<OffsetCnt>(_ctrlDir, dirName, [this] (uint_fast32_t offset, uint_fast32_t cnt) {
                        _fsUtils->addFilesToDir(
                                _ctrlDir,
                                _fileObtainer->getMyAudios(offset, cnt),
                                _idGenerator,
                                _settings->getMp3Ext()
                        );
                    });
                } else {
                    throw FsException("Can't create non-counter, non-refresh dir in My Audios dir");
                }
            }

            void deleteFile(const std::string &path) {
                FsPath fsPath = _fsUtils->findPath(_ctrlDir, _fsUtils->stripPathPrefix(path, DIR_NAME), FsPath::WITH_PARENT_DIR);
                FsPathUnlocker unlocker{fsPath};
                if(!fsPath.isPathMatched() || !fsPath.getAll().back().isFile()){
                    throw FsException("File does not exist " + path);
                }
                auto file = fsPath.getAll().back();
                auto remoteFile = std::get<RemoteFile>(*file.file()->getExtra());
                _fileObtainer->deleteFromMyAudios(remoteFile.getOwnerId(), remoteFile.getFileId());
                fsPath.getAll().front().dir()->removeItem(file.getName());
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
            ActTuple<TFsUtils> _acts;
        };
    }
}
