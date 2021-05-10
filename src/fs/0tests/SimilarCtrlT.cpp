#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fs/ctrl/SimilarCtrl.h>
#include <diext/common_di.h>
#include <fs/FsUtils.h>
#include <fs/AsyncFsManager.h>
#include "data/FsHelper.h"
#include "data/FileCacheM.h"
#include "data/PhotoCacheM.h"
#include "data/RealFsM.h"
#include "data/ThreadPoolM.h"
#include "data/FileObtainerM.h"
#include "data/CtrlM.h"

namespace di = boost::di;

class SimilarCtrlT: public ::testing::Test, public FsHelper {
public:
    using FsPath = vk_music_fs::fs::FsPath;
    using FsPathUnlocker = vk_music_fs::fs::FsPathUnlocker;

    using SimilarCtrl = vk_music_fs::fs::SimilarCtrl<
            CtrlM,
            vk_music_fs::fs::FsUtils, FileObtainerM,
            vk_music_fs::fs::AsyncFsManager<vk_music_fs::fs::FsUtils, FileCacheM, PhotoCacheM, RealFsM, ThreadPoolM>
            >;
    using OffsetCntPlaylist = vk_music_fs::fs::OffsetCntPlaylist;

    auto_init(inj, (vk_music_fs::makeStorageInj(
            di::bind<vk_music_fs::Mp3Extension>.to(vk_music_fs::Mp3Extension{".mp3"}),
            di::bind<vk_music_fs::NumSearchFiles>.to(vk_music_fs::NumSearchFiles{2}),
            di::bind<vk_music_fs::CreateDummyDirs>.to(vk_music_fs::CreateDummyDirs{false}),
            di::bind<vk_music_fs::fs::UseAsyncNotifier>.to(vk_music_fs::fs::UseAsyncNotifier{false}),
            di::bind<vk_music_fs::fs::PhotoName>.to(vk_music_fs::fs::PhotoName{"photo.jpg"}),
            di::bind<vk_music_fs::fs::PathToFs>.to(vk_music_fs::fs::PathToFs{"/"})
            )));
    std::shared_ptr<SimilarCtrl> ctrl;
    std::shared_ptr<CtrlM> childCtrl;
    std::shared_ptr<vk_music_fs::fs::FsUtils> fsUtils;
    vk_music_fs::fs::DirPtr ctrlDir;
    std::shared_ptr<FileObtainerM> obtainer;
protected:
    void SetUp() override {
        ctrl = inj.create<std::shared_ptr<SimilarCtrl>>();
        childCtrl = inj.create<std::shared_ptr<CtrlM>>();
        fsUtils = inj.create<std::shared_ptr<vk_music_fs::fs::FsUtils>>();
        obtainer = inj.create<std::shared_ptr<FileObtainerM>>();
        ctrlDir = std::make_shared<vk_music_fs::fs::Dir>("/test", 0, std::nullopt, vk_music_fs::fs::DirWPtr{});
        ctrlDir->addItem(std::make_shared<vk_music_fs::fs::File>(
                "Justin Bieber - Baby.mp3", 567, 0,
                vk_music_fs::RemoteFile{"http://url1", 1, 2, "Justin Bieber", "Baby"},
                ctrlDir
        ));
        ON_CALL(*childCtrl, getCtrlDir()).WillByDefault(testing::Return(ctrlDir));
        ON_CALL(*childCtrl, getDirName()).WillByDefault(testing::Return("test"));
    }
};

TEST_F(SimilarCtrlT, CreateSimilarDir){
    FsPath oldFsPath = fsUtils->findPath(ctrlDir, "Justin Bieber - Baby.mp3", FsPath::WITH_PARENT_DIR);
    FsPath newFsPath = fsUtils->findPath(ctrlDir, "Justin Bieber - Baby_s.mp3", {oldFsPath}, FsPath::WITH_PARENT_DIR);
    std::vector<vk_music_fs::RemoteFile> simFileData{{"http://url2", 1, 4, "Artist1", "Song1"},
                                                       {"http://url3", 1, 5, "Artist2", "Song2"}};
    EXPECT_CALL(*obtainer, searchSimilar(vk_music_fs::RemoteFileId{1, 2}, 0, 2))
        .WillOnce(testing::Return(simFileData));
    ctrl->rename(oldFsPath, newFsPath);
    std::set<std::string> exp = {"Artist1 - Song1.mp3", "Artist2 - Song2.mp3"};
    EXPECT_EQ(listContents(ctrlDir->getItem("Justin Bieber - Baby").dir()), exp);
}

TEST_F(SimilarCtrlT, AddMoreSimilarFiles){
    {
        FsPath oldFsPath = fsUtils->findPath(ctrlDir, "Justin Bieber - Baby.mp3", FsPath::WITH_PARENT_DIR);
        FsPath newFsPath = fsUtils->findPath(ctrlDir, "Justin Bieber - Baby_s.mp3", {oldFsPath}, FsPath::WITH_PARENT_DIR);
        FsPathUnlocker unlocker{std::vector{oldFsPath, newFsPath}};
        std::vector<vk_music_fs::RemoteFile> simFileData{{"http://url2", 1, 4, "Artist1", "Song1"},
                                                         {"http://url3", 1, 5, "Artist2", "Song2"}};
        EXPECT_CALL(*obtainer, searchSimilar(vk_music_fs::RemoteFileId{1, 2}, 0, 2))
                .WillOnce(testing::Return(simFileData));
        ctrl->rename(oldFsPath, newFsPath);
    }

    {
        FsPath dirPath = fsUtils->findPath(ctrlDir, "Justin Bieber - Baby/4", FsPath::WITH_PARENT_DIR);
        std::vector<vk_music_fs::RemoteFile> simFileData{{"http://url2", 1, 6, "Artist3", "Song3"},
                                                         {"http://url3", 1, 7, "Artist4", "Song4"}};
        EXPECT_CALL(*obtainer, searchSimilar(vk_music_fs::RemoteFileId{1, 2}, 2, 2))
                .WillOnce(testing::Return(simFileData));
        ctrl->createDir(dirPath);
        std::set<std::string> exp = {"4", "Artist1 - Song1.mp3", "Artist2 - Song2.mp3", "Artist3 - Song3.mp3", "Artist4 - Song4.mp3"};
        EXPECT_EQ(listContents(ctrlDir->getItem("Justin Bieber - Baby").dir()), exp);
    }
}

TEST_F(SimilarCtrlT, DeleteSimilarDir){
    {
        FsPath oldFsPath = fsUtils->findPath(ctrlDir, "Justin Bieber - Baby.mp3", FsPath::WITH_PARENT_DIR);
        FsPath newFsPath = fsUtils->findPath(ctrlDir, "Justin Bieber - Baby_s.mp3", {oldFsPath}, FsPath::WITH_PARENT_DIR);
        FsPathUnlocker unlocker{std::vector{oldFsPath, newFsPath}};
        std::vector<vk_music_fs::RemoteFile> simFileData{{"http://url2", 1, 4, "Artist1", "Song1"},
                                                         {"http://url3", 1, 5, "Artist2", "Song2"}};
        EXPECT_CALL(*obtainer, searchSimilar(vk_music_fs::RemoteFileId{1, 2}, 0, 2))
                .WillOnce(testing::Return(simFileData));
        ctrl->rename(oldFsPath, newFsPath);
    }
    ctrl->deleteDir("test/Justin Bieber - Baby");
    std::set<std::string> exp = {"Justin Bieber - Baby_s.mp3"};
    EXPECT_EQ(listContents(ctrlDir), exp);
}

TEST_F(SimilarCtrlT, DeleteSimilarFile){
    {
        FsPath oldFsPath = fsUtils->findPath(ctrlDir, "Justin Bieber - Baby.mp3", FsPath::WITH_PARENT_DIR);
        FsPath newFsPath = fsUtils->findPath(ctrlDir, "Justin Bieber - Baby_s.mp3", {oldFsPath}, FsPath::WITH_PARENT_DIR);
        FsPathUnlocker unlocker{std::vector{oldFsPath, newFsPath}};
        std::vector<vk_music_fs::RemoteFile> simFileData{{"http://url2", 1, 4, "Artist1", "Song1"},
                                                         {"http://url3", 1, 5, "Artist2", "Song2"}};
        EXPECT_CALL(*obtainer, searchSimilar(vk_music_fs::RemoteFileId{1, 2}, 0, 2))
                .WillOnce(testing::Return(simFileData));
        ctrl->rename(oldFsPath, newFsPath);
    }
    ctrl->deleteFile("test/Justin Bieber - Baby/Artist1 - Song1.mp3");
    std::set<std::string> exp = {"Artist2 - Song2.mp3"};
    EXPECT_EQ(listContents(ctrlDir->getItem("Justin Bieber - Baby").dir()), exp);
}