#pragma once
#include <fs/common_fs.h>
#include <fs/FsSettings.h>
#include <fs/Dir.h>
#include <fs/File.h>
#include <fs/IdGenerator.h>
#include "ThrowExCtrl.h"
#include <fs/actions/act.h>
#include <fs/OffsetCntPlaylist.h>
#include <regex>
#include <fs/FileName.h>

namespace vk_music_fs {
    namespace fs {
        template <typename TFsUtils, typename TFileObtainer, typename TAsyncFsManager>
        class PlaylistCtrl : public ThrowExCtrl{
        public:
            PlaylistCtrl(
                    const std::shared_ptr<TFsUtils> &utils,
                    const std::shared_ptr<TFileObtainer> &fileObtainer,
                    const std::shared_ptr<FsSettings> &settings,
                    const std::shared_ptr<IdGenerator> &idGenerator,
                    const ActTuple<TFsUtils> &acts,
                    const std::shared_ptr<TAsyncFsManager> &asyncFsManager
            ) : _fsUtils(utils), _fileObtainer(fileObtainer), _idGenerator(idGenerator),
                _settings(settings), _acts(acts), _asyncFsManager(asyncFsManager) {
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
                if(
                        parent.getId() != _ctrlDir->getId() &&
                        (!parent.dir()->getDirExtra() || !std::holds_alternative<OffsetCntPlaylist>(*parent.dir()->getDirExtra()))
                ){
                    throw FsException("Can't create dir inside counter dir");
                }
            }

            void createDir(FsPath &path) {
                checkCreateDirPath(path);
                auto dirName = path.getStringParts().back();
                auto parent = path.getAll().back().dir();
                QueryParams query = _fsUtils->parseQuery(dirName);
                if(parent->getId() == _ctrlDir->getId()){
                    if(query.type == QueryParams::Type::TWO_NUMBERS || query.type == QueryParams::Type::ONE_NUMBER){
                        getAct<RemoveRefreshDirAct>(_acts)->template doAction<OffsetCnt>(
                            _ctrlDir,
                            [this, dirName, query] {
                                getAct<NumberAct>(_acts)->template doAction<OffsetCnt>(
                                        _ctrlDir, dirName, query,
                                        _fsUtils->getAllDeleter(),
                                        [this] (uint_fast32_t offset, uint_fast32_t cnt) {
                                            addPlaylistsToDir(
                                                    _ctrlDir,
                                                    _fileObtainer->getMyPlaylists(offset, cnt)
                                            );
                                        }
                                );
                            }
                        );
                    } else if(_fsUtils->isRefreshDir(dirName)){
                        getAct<RefreshAct>(_acts)->template doAction<OffsetCnt>(
                            _ctrlDir, dirName,
                            _fsUtils->template getCounterDirLeaver<OffsetCnt>(),
                            [this] (uint_fast32_t offset, uint_fast32_t cnt) {
                                addPlaylistsToDir(
                                        _ctrlDir,
                                        _fileObtainer->getMyPlaylists(offset, cnt)
                                );
                            }
                        );
                    } else {
                        throw FsException("Can't create non-counter, non-refresh dir in My Playlists dir");
                    }
                } else {
                    if(query.type == QueryParams::Type::TWO_NUMBERS || query.type == QueryParams::Type::ONE_NUMBER){
                        getAct<RemoveRefreshDirAct>(_acts)->template doAction<OffsetCntPlaylist>(
                            parent,
                            [this, dirName, parent, query] {
                                getAct<NumberAct>(_acts)->template doAction<OffsetCntPlaylist>(
                                        parent, dirName, query,
                                        _fsUtils->getAllDeleter(),
                                        [this, parent] (uint_fast32_t offset, uint_fast32_t cnt) {
                                            OffsetCntPlaylist curOffsetCntPlaylist = std::get<OffsetCntPlaylist>(*parent->getDirExtra());
                                            auto playlist = curOffsetCntPlaylist.getPlaylist();
                                            _asyncFsManager->createFiles(
                                                    parent,
                                                    _fileObtainer->getPlaylistAudios(
                                                            playlist.accessKey, playlist.ownerId, playlist.albumId,
                                                            offset, cnt
                                                    )
                                            );
                                        }
                                );
                            }
                        );
                    } else if(_fsUtils->isRefreshDir(dirName)){
                        getAct<RefreshAct>(_acts)->template doAction<OffsetCntPlaylist>(
                            parent, dirName,
                            _fsUtils->template getCounterDirLeaver<OffsetCntPlaylist>(),
                            [this, parent] (uint_fast32_t offset, uint_fast32_t cnt) {
                                OffsetCntPlaylist curOffsetCntPlaylist = std::get<OffsetCntPlaylist>(
                                        *parent->getDirExtra());
                                auto playlist = curOffsetCntPlaylist.getPlaylist();
                                _asyncFsManager->createFiles(
                                        parent,
                                        _fileObtainer->getPlaylistAudios(
                                                playlist.accessKey, playlist.ownerId,
                                                playlist.albumId, offset, cnt
                                        )
                                );
                            }
                        );
                    } else {
                        throw FsException("Can't create non-counter, non-refresh dir in My Playlists dir");
                    }
                }
            }

            void deleteFile(FsPath &fsPath) {
                getAct<DeleteFileAct>(_acts)->doAction(fsPath);
            }

            void deleteDir(FsPath &fsPath) {
                if(!fsPath.isPathMatched() || !fsPath.getAll().back().isDir()){
                    throw FsException("Dir does not exist " + fsPath.getStringParts().back());
                }
                auto dir = fsPath.getAll().back();
                auto dirName = fsPath.getStringParts().back();
                auto parent = dir.dir()->getParent();
                if (parent->getId() == _ctrlDir->getId()) {
                    OffsetCntPlaylist curOffsetCntPlaylist = std::get<OffsetCntPlaylist>(*dir.dir()->getDirExtra());
                    auto playlist = curOffsetCntPlaylist.getPlaylist();
                    _fileObtainer->deletePlaylist(playlist.ownerId, playlist.albumId);
                }
                getAct<DeleteDirAct>(_acts)->doAction(fsPath, [] () {return false;});
            }

            std::string getDirName(){
                return DIR_NAME;
            }

        private:
            void addPlaylistsToDir(const DirPtr &dir, const std::vector<PlaylistData> &playlists){
                auto curTime = dir->getNumFiles();
                for(const auto &playlist: playlists){
                    auto fname = FileName(playlist.title);
                    while (dir->hasItem(fname.getFilename())) {
                        fname.increaseNumberSuffix();
                    }
                    auto newFile = std::make_shared<Dir>(
                            fname.getFilename(),
                            _idGenerator->getNextId(),
                            OffsetCntPlaylist{0, 0, DirPtr{}, DirPtr{}, playlist},
                            dir,
                            curTime
                    );
                    dir->addItem(newFile);
                    curTime++;
                }
            }

            constexpr const static char *DIR_NAME = "My playlists";
            std::shared_ptr<TFsUtils> _fsUtils;
            std::shared_ptr<TFileObtainer> _fileObtainer;
            std::shared_ptr<IdGenerator> _idGenerator;
            std::shared_ptr<FsSettings> _settings;
            std::shared_ptr<TAsyncFsManager> _asyncFsManager;
            DirPtr _ctrlDir;
            ActTuple<TFsUtils> _acts;
        };
    }
}