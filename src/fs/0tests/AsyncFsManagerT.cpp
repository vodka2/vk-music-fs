#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fs/AsyncFsManager.h>
#include <diext/common_di.h>
#include <fs/FsUtils.h>
#include <fs/Dir.h>
#include "data/FileCacheM.h"
#include "data/PhotoCacheM.h"
#include "data/FsHelper.h"
#include "data/RealFsM.h"
#include "data/ThreadPoolM.h"

namespace di = boost::di;

class AsyncFsManagerT: public ::testing::Test, public FsHelper {
public:
    typedef vk_music_fs::fs::AsyncFsManager<vk_music_fs::fs::FsUtils, FileCacheM, PhotoCacheM, RealFsM, ThreadPoolM> AsyncFsManager;
    AsyncFsManagerT() {}

    auto_init(inj, (vk_music_fs::makeStorageInj(
            di::bind<vk_music_fs::Mp3Extension>.to(vk_music_fs::Mp3Extension{".mp3"}),
            di::bind<vk_music_fs::NumSearchFiles>.to(vk_music_fs::NumSearchFiles{5}),
            di::bind<vk_music_fs::CreateDummyDirs>.to(vk_music_fs::CreateDummyDirs{false}),
            di::bind<vk_music_fs::fs::UseAsyncNotifier>.to(vk_music_fs::fs::UseAsyncNotifier{true}),
            di::bind<vk_music_fs::fs::PhotoName>.to(vk_music_fs::fs::PhotoName{"photo.jpg"}),
            di::bind<vk_music_fs::fs::PathToFs>.to(vk_music_fs::fs::PathToFs{"testRoot"})
    )));

    std::shared_ptr<AsyncFsManager> fsManager;
    std::shared_ptr<FileCacheM> fileCache;
    std::shared_ptr<PhotoCacheM> photoCache;
    std::shared_ptr<RealFsM> realFs;
    vk_music_fs::fs::DirPtr dir;

    void SetUp() override {
        using vk_music_fs::fs::DirPtr;
        fsManager = inj.create<std::shared_ptr<AsyncFsManager>>();
        fileCache = inj.create<std::shared_ptr<FileCacheM>>();
        photoCache = inj.create<std::shared_ptr<PhotoCacheM>>();
        realFs = inj.create<std::shared_ptr<RealFsM>>();
        dir = std::make_shared<vk_music_fs::fs::Dir>(
                "test", 0, vk_music_fs::fs::OffsetCnt{0,0,DirPtr{},DirPtr{}},
                vk_music_fs::fs::DirWPtr{}
        );
    }
};

TEST_F(AsyncFsManagerT, CreateFiles){
    std::vector<vk_music_fs::RemoteFile> files{{"http://url1", 1, 2, "Art 1", "Title1"},
                                               {"http://url2", 6, 7, "Art 2", "Title 2"}};
    EXPECT_CALL(*fileCache, getFileSize(files[0]));
    EXPECT_CALL(*fileCache, getFileSize(files[1]));
    EXPECT_CALL(*realFs, createFile("testRoot/test/Art 1 - Title1.mp3"));
    EXPECT_CALL(*realFs, createFile("testRoot/test/Art 2 - Title 2.mp3"));
    fsManager->createFiles(dir, files);
    std::set<std::string> exp = {"Art 1 - Title1.mp3", "Art 2 - Title 2.mp3"};
    EXPECT_EQ(listContents(dir), exp);
}

TEST_F(AsyncFsManagerT, CreatePhoto){
    vk_music_fs::RemotePhotoFile RemotePhotoFile{"hello", -22, 33};
    EXPECT_CALL(*photoCache, getFileSize(RemotePhotoFile));
    EXPECT_CALL(*realFs, createFile("testRoot/test/photo.jpg"));
    fsManager->createPhoto(dir, RemotePhotoFile);
    std::set<std::string> exp = {"photo.jpg"};
    EXPECT_EQ(listContents(dir), exp);
}