#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fs/ctrl/PlaylistCtrl.h>
#include <diext/common_di.h>
#include <fs/FsUtils.h>
#include "data/FsHelper.h"

namespace di = boost::di;

class FileObtainerM0{
public:
    template <typename... T>
    FileObtainerM0(T&&... args){}//NOLINT
    MOCK_CONST_METHOD2(getMyPlaylists, std::vector<vk_music_fs::fs::PlaylistData>(uint_fast32_t offset, uint_fast32_t));
    MOCK_CONST_METHOD5(
            getPlaylistAudios,
            std::vector<vk_music_fs::RemoteFile>(
               const std::string &, int_fast32_t, uint_fast32_t, uint_fast32_t, uint_fast32_t
            )
    );

};

typedef testing::NiceMock<FileObtainerM0> FileObtainerM;

class PlaylistCtrlT: public ::testing::Test, public FsHelper {
public:
    using PlaylistCtrl = vk_music_fs::fs::PlaylistCtrl<vk_music_fs::fs::FsUtils, FileObtainerM>;
    using OffsetCntPlaylist = vk_music_fs::fs::OffsetCntPlaylist;

    auto_init(inj, (vk_music_fs::makeStorageInj(
            di::bind<vk_music_fs::Mp3Extension>.to(vk_music_fs::Mp3Extension{".mp3"}),
            di::bind<vk_music_fs::NumSearchFiles>.to(vk_music_fs::NumSearchFiles{5}),
            di::bind<vk_music_fs::CreateDummyDirs>.to(vk_music_fs::CreateDummyDirs{false})
            )));
    std::shared_ptr<PlaylistCtrl> ctrl;
    std::shared_ptr<vk_music_fs::fs::FsUtils> fsUtils;
    vk_music_fs::fs::DirPtr rootDir;
    vk_music_fs::fs::DirPtr ctrlDir;
    std::shared_ptr<FileObtainerM> obtainer;
protected:
    void SetUp() override {
        ctrl = inj.create<std::shared_ptr<PlaylistCtrl>>();
        fsUtils = inj.create<std::shared_ptr<vk_music_fs::fs::FsUtils>>();
        obtainer = inj.create<std::shared_ptr<FileObtainerM>>();
        rootDir = std::make_shared<vk_music_fs::fs::Dir>("/", 0, std::nullopt, vk_music_fs::fs::DirWPtr{});
        ctrl->setRootDir(rootDir);
        ctrlDir = ctrl->getCtrlDir();
    }
};

TEST_F(PlaylistCtrlT, CreatePlaylistDir) {
    auto path = fsUtils->findPath(ctrlDir, "10", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    std::vector<vk_music_fs::fs::PlaylistData> plData{{33, 1, "11", "pl1"}, {1, 55, "12", "pl2"}};
    ON_CALL(*obtainer, getMyPlaylists(0, 10)).WillByDefault(testing::Return(plData));
    ctrl->createDir(path);
    EXPECT_EQ(std::get<OffsetCntPlaylist>(*ctrlDir->getItem("pl2").dir()->getDirExtra()).getPlaylist().albumId, 55);
    EXPECT_EQ(std::get<OffsetCntPlaylist>(*ctrlDir->getItem("pl1").dir()->getDirExtra()).getPlaylist().accessKey, "11");
}

TEST_F(PlaylistCtrlT, CreatePlaylistDirDuplicates) {
    auto path = fsUtils->findPath(ctrlDir, "10", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    std::vector<vk_music_fs::fs::PlaylistData> plData{{3, 1, "11", "pl1"}, {12, 15, "12", "pl1"}, {1, 56, "13", "pl1"}};
    ON_CALL(*obtainer, getMyPlaylists(0, 10)).WillByDefault(testing::Return(plData));
    ctrl->createDir(path);
    std::set<std::string> exp = {"10", "pl1", "pl1_2", "pl1_3"};
    EXPECT_EQ(listContents(ctrlDir), exp);
}

TEST_F(PlaylistCtrlT, CreateFewerPlaylistDirs) {
    std::vector<vk_music_fs::fs::PlaylistData> plData{{3, 1, "11", "pl1"}, {12, 15, "12", "pl1"}, {1, 56, "13", "pl1"}};
    ON_CALL(*obtainer, getMyPlaylists(0, 10)).WillByDefault(testing::Return(plData));
    auto fullPath = fsUtils->findPath(ctrlDir, "10", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    ctrl->createDir(fullPath);
    fullPath.unlockAll();
    auto samePath = fsUtils->findPath(ctrlDir, "3", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    ctrl->createDir(samePath);
    samePath.unlockAll();
    auto fewerPath = fsUtils->findPath(ctrlDir, "1", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    ctrl->createDir(fewerPath);
    fewerPath.unlockAll();

    std::set<std::string> exp = {"1", "pl1"};
    EXPECT_EQ(listContents(ctrlDir), exp);
    EXPECT_EQ(std::get<OffsetCntPlaylist>(*ctrlDir->getItem("pl1").dir()->getDirExtra()).getPlaylist().ownerId, 3);
}

TEST_F(PlaylistCtrlT, RefreshPlaylistDir) {
    std::vector<vk_music_fs::fs::PlaylistData> plData{{3, 1, "11", "pl1"}, {12, 15, "12", "pl1"}};
    std::vector<vk_music_fs::fs::PlaylistData> plRefreshData{{31, 21, "12", "pl8"}, {178, 17, "13", "pl9"}};
    EXPECT_CALL(*obtainer, getMyPlaylists(0, 10)).WillOnce(testing::Return(plData)).WillOnce(testing::Return(plRefreshData));
    auto fullPath = fsUtils->findPath(ctrlDir, "10", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    ctrl->createDir(fullPath);
    fullPath.unlockAll();
    auto refreshPath = fsUtils->findPath(ctrlDir, "refresh", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    ctrl->createDir(refreshPath);
    refreshPath.unlockAll();

    EXPECT_EQ(std::get<OffsetCntPlaylist>(*ctrlDir->getItem("pl8").dir()->getDirExtra()).getPlaylist().albumId, 21);
    EXPECT_EQ(std::get<OffsetCntPlaylist>(*ctrlDir->getItem("pl9").dir()->getDirExtra()).getPlaylist().accessKey, "13");
}

TEST_F(PlaylistCtrlT, CreatePlaylistAudiosDir) {
    std::vector<vk_music_fs::fs::PlaylistData> plData{{3, 1, "11", "pl1"}, {12, 15, "12", "pl2"}};
    std::vector<vk_music_fs::RemoteFile> files{{"http://url1", 1, 2, "Art 1", "Title1"}, {"http://url2", 6, 7, "Art 2", "Title 2"}};
    EXPECT_CALL(*obtainer, getMyPlaylists(0, 10)).WillOnce(testing::Return(plData));
    EXPECT_CALL(*obtainer, getPlaylistAudios("12", 12, 15, 0, 7)).WillOnce(testing::Return(files));
    auto path = fsUtils->findPath(ctrlDir, "10", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    ctrl->createDir(path);
    path.unlockAll();
    auto filesPath = fsUtils->findPath(ctrlDir, "pl2/7", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    ctrl->createDir(filesPath);
    filesPath.unlockAll();

    std::set<std::string> exp = {"7", "Art 1 - Title1.mp3", "Art 2 - Title 2.mp3"};
    EXPECT_EQ(listContents(filesPath.getLast().dir()), exp);

    EXPECT_EQ(
            std::get<vk_music_fs::RemoteFile>(*filesPath.getLast().dir()->getItem("Art 2 - Title 2.mp3").file()->getExtra()),
            files.back()
    );
}

TEST_F(PlaylistCtrlT, CreatePlaylistAudiosDirTwice) {
    std::vector<vk_music_fs::fs::PlaylistData> plData{{3, 1, "11", "pl1"}, {12, 15, "12", "pl2"}};
    std::vector<vk_music_fs::RemoteFile> files{{"http://url1", 1, 2, "Art 1", "Title1"}, {"http://url2", 6, 7, "Art 2", "Title 2"}};
    std::vector<vk_music_fs::RemoteFile> files2{{"http://url3", 13, 21, "Art 3", "Title 3"}};
    EXPECT_CALL(*obtainer, getMyPlaylists(0, 10)).WillOnce(testing::Return(plData));
    EXPECT_CALL(*obtainer, getPlaylistAudios("12", 12, 15, 0, 2)).WillOnce(testing::Return(files));
    EXPECT_CALL(*obtainer, getPlaylistAudios("12", 12, 15, 2, 1)).WillOnce(testing::Return(files2));
    auto path = fsUtils->findPath(ctrlDir, "10", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    ctrl->createDir(path);
    path.unlockAll();
    auto filesPath = fsUtils->findPath(ctrlDir, "pl2/2", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    ctrl->createDir(filesPath);
    filesPath.unlockAll();
    auto files2Path = fsUtils->findPath(ctrlDir, "pl2/3", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    ctrl->createDir(files2Path);
    files2Path.unlockAll();

    std::set<std::string> exp = {"3", "Art 1 - Title1.mp3", "Art 2 - Title 2.mp3", "Art 3 - Title 3.mp3"};
    EXPECT_EQ(listContents(files2Path.getLast().dir()), exp);
}

TEST_F(PlaylistCtrlT, RefreshPlaylistAudiosDir) {
    std::vector<vk_music_fs::fs::PlaylistData> plData{{3, 1, "11", "pl1"}, {12, 15, "12", "pl2"}, {12, 15, "12", "pl3"}};
    std::vector<vk_music_fs::RemoteFile> files{{"http://url1", 1, 2, "Art 1", "Title1"}, {"http://url2", 6, 7, "Art 2", "Title 2"}};
    std::vector<vk_music_fs::RemoteFile> files2{{"http://url3", 13, 21, "Art 3", "Title 3"}};
    EXPECT_CALL(*obtainer, getMyPlaylists(0, 10)).WillOnce(testing::Return(plData));
    EXPECT_CALL(*obtainer, getPlaylistAudios("11", 3, 1, 1, 5)).WillOnce(testing::Return(files)).WillOnce(testing::Return(files2));
    auto path = fsUtils->findPath(ctrlDir, "10", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    ctrl->createDir(path);
    path.unlockAll();
    auto filesPath = fsUtils->findPath(ctrlDir, "pl1/1-5", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    ctrl->createDir(filesPath);
    filesPath.unlockAll();
    auto refreshPath = fsUtils->findPath(ctrlDir, "pl1/r", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    ctrl->createDir(refreshPath);
    refreshPath.unlockAll();

    std::set<std::string> exp = {"r", "Art 3 - Title 3.mp3"};
    EXPECT_EQ(listContents(refreshPath.getLast().dir()), exp);
    EXPECT_EQ(std::get<OffsetCntPlaylist>(*ctrlDir->getItem("pl1").dir()->getDirExtra()).getRefreshDir()->getName(), "r");
}


TEST_F(PlaylistCtrlT, RefreshPlaylistAudiosDirThenReload) {
    std::vector<vk_music_fs::fs::PlaylistData> plData{{3, 1, "11", "pl1"}, {12, 15, "12", "pl2"}, {12, 15, "12", "pl3"}};
    std::vector<vk_music_fs::RemoteFile> files{{"http://url1", 1, 2, "Art 1", "Title1"}, {"http://url2", 6, 7, "Art 2", "Title 2"}};
    std::vector<vk_music_fs::RemoteFile> files2{{"http://url3", 13, 21, "Art 3", "Title 3"}};
    EXPECT_CALL(*obtainer, getMyPlaylists(0, 10)).WillOnce(testing::Return(plData));
    EXPECT_CALL(*obtainer, getPlaylistAudios("11", 3, 1, 1, 5)).WillOnce(testing::Return(files)).WillOnce(testing::Return(files2));
    EXPECT_CALL(*obtainer, getPlaylistAudios("11", 3, 1, 2, 4)).WillOnce(testing::Return(files));
    auto path = fsUtils->findPath(ctrlDir, "10", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    ctrl->createDir(path);
    path.unlockAll();
    auto filesPath = fsUtils->findPath(ctrlDir, "pl1/1-5", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    ctrl->createDir(filesPath);
    filesPath.unlockAll();
    auto refreshPath = fsUtils->findPath(ctrlDir, "pl1/r", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    ctrl->createDir(refreshPath);
    refreshPath.unlockAll();
    auto files2Path = fsUtils->findPath(ctrlDir, "pl1/2-4", vk_music_fs::fs::FsPath::WITH_PARENT_DIR);
    ctrl->createDir(files2Path);
    files2Path.unlockAll();

    std::set<std::string> exp = {"2-4", "Art 1 - Title1.mp3", "Art 2 - Title 2.mp3"};
    EXPECT_EQ(listContents(files2Path.getLast().dir()), exp);
    EXPECT_EQ(std::get<OffsetCntPlaylist>(*ctrlDir->getItem("pl1").dir()->getDirExtra()).getRefreshDir(), nullptr);
}