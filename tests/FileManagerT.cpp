#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/di.hpp>
#include <boost/di/extension/scopes/scoped.hpp>
#include <FileManager.h>

using vk_music_fs::ByteVect;
using vk_music_fs::InjPtr;
using vk_music_fs::RemoteFile;
using vk_music_fs::FNameCache;
namespace di = boost::di;

class VkApiM{
public:
    VkApiM(){} //NOLINT
    MOCK_CONST_METHOD1(getRemoteFile, RemoteFile(const std::string &str));
};

class FileCacheM{
public:
    FileCacheM(){} //NOLINT
    MOCK_CONST_METHOD1(getFilename, FNameCache(const RemoteFile &f));
    MOCK_CONST_METHOD1(fileClosed, void(const std::string &str));
    MOCK_CONST_METHOD1(getFileSize, uint_fast32_t(const RemoteFile &file));
    MOCK_CONST_METHOD1(getTagSize, uint_fast32_t(const RemoteFile &file));
};

class FileProcessorM{
public:
    FileProcessorM(){} //NOLINT
    MOCK_CONST_METHOD0(openBlocking, void());
    MOCK_CONST_METHOD2(read, ByteVect(uint_fast32_t offset, uint_fast32_t size));
};

class ReaderM{
public:
    ReaderM(){} //NOLINT
    MOCK_CONST_METHOD0(openBlocking, void());
    MOCK_CONST_METHOD2(read, ByteVect(uint_fast32_t offset, uint_fast32_t size));
};

typedef vk_music_fs::FileManager<VkApiM, FileCacheM, FileProcessorM, ReaderM> FileManager;


class FileManagerT: public ::testing::Test {
public:
    InjPtr<
            std::shared_ptr<FileProcessorM>,
            std::shared_ptr<ReaderM>
    > mainInj = std::make_shared<di::injector<
            std::shared_ptr<FileProcessorM>,
            std::shared_ptr<ReaderM>
    >>(di::make_injector(
            di::bind<FileProcessorM>.in(di::extension::scoped),
            di::bind<ReaderM>.in(di::extension::scoped)
    ));

    di::injector<
    std::shared_ptr<FileManager>,
    std::shared_ptr<VkApiM>,
    std::shared_ptr<FileCacheM>,
    InjPtr<
        std::shared_ptr<FileProcessorM>,
        std::shared_ptr<ReaderM>
    >> inj = di::make_injector(
            di::bind<FileManager>.in(di::extension::scoped),
            di::bind<VkApiM>.in(di::extension::scoped),
            di::bind<FileCacheM>.in(di::extension::scoped),
            di::bind<InjPtr<
                    std::shared_ptr<FileProcessorM>,
                    std::shared_ptr<ReaderM>
            >>.to(mainInj)
    );

    std::string file = "/baby.mp3";
    std::string file2 = "/baby2.mp3";
    std::string cachedFile = "/cachedBaby.mp3";
    std::string cachedFile2 = "/cachedBaby2.mp3";
    RemoteFile rf{"http://example.net/", 0, 0, "Justin Bieber", "Baby"};
    RemoteFile rf2{"http://example2.net/", 2, 3, "Justin Bieber", "Baby"};
    ByteVect fileContents{0, 1, 2, 3, 4};
    ByteVect fileContents2{0, 1, 2, 3, 4, 5};
};

TEST_F(FileManagerT, OpenFileCache){ //NOLINT
    auto t = inj.create<std::shared_ptr<FileManager>>();
    EXPECT_CALL(*inj.create<std::shared_ptr<VkApiM>>(), getRemoteFile(file)).WillOnce(testing::Return(rf));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getFilename(rf))
        .WillOnce(testing::Return(FNameCache{cachedFile, true}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), fileClosed(cachedFile));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getTagSize(rf)).Times(0);
    t->close(static_cast<uint_fast32_t>(t->open(file)));
}

TEST_F(FileManagerT, OpenFileNoCache){ //NOLINT
    auto t = inj.create<std::shared_ptr<FileManager>>();
    EXPECT_CALL(*inj.create<std::shared_ptr<VkApiM>>(), getRemoteFile(file)).WillOnce(testing::Return(rf));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getFilename(rf))
        .WillOnce(testing::Return(FNameCache{cachedFile, false}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), fileClosed(cachedFile));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getTagSize(rf));
    t->close(static_cast<uint_fast32_t>(t->open(file)));
}

TEST_F(FileManagerT, OpenFileCacheRead){ //NOLINT
    auto t = inj.create<std::shared_ptr<FileManager>>();
    EXPECT_CALL(*inj.create<std::shared_ptr<VkApiM>>(), getRemoteFile(file)).WillOnce(testing::Return(rf));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getFilename(rf))
            .WillOnce(testing::Return(FNameCache{cachedFile, true}));
    EXPECT_CALL(*mainInj->create<std::shared_ptr<ReaderM>>(), openBlocking());
    EXPECT_CALL(*mainInj->create<std::shared_ptr<ReaderM>>(), read(10, 200)).WillOnce(testing::Return(fileContents));
    auto id = static_cast<uint_fast32_t>(t->open(file));
    EXPECT_EQ(t->read(id, 10, 200), fileContents);
    t->close(id);
}

TEST_F(FileManagerT, OpenFileNoCacheRead){ //NOLINT
    auto t = inj.create<std::shared_ptr<FileManager>>();
    EXPECT_CALL(*inj.create<std::shared_ptr<VkApiM>>(), getRemoteFile(file)).WillOnce(testing::Return(rf));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getFilename(rf))
            .WillOnce(testing::Return(FNameCache{cachedFile, false}));
    EXPECT_CALL(*mainInj->create<std::shared_ptr<FileProcessorM>>(), openBlocking());
    EXPECT_CALL(*mainInj->create<std::shared_ptr<FileProcessorM>>(), read(10, 200)).WillOnce(testing::Return(fileContents));
    auto id = static_cast<uint_fast32_t>(t->open(file));
    EXPECT_EQ(t->read(id, 10, 200), fileContents);
    t->close(id);
}

TEST_F(FileManagerT, OpenFileCacheRead2Times){ //NOLINT
    auto t = inj.create<std::shared_ptr<FileManager>>();
    EXPECT_CALL(*inj.create<std::shared_ptr<VkApiM>>(), getRemoteFile(file)).WillOnce(testing::Return(rf));
    EXPECT_CALL(*inj.create<std::shared_ptr<VkApiM>>(), getRemoteFile(file2)).WillOnce(testing::Return(rf2));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getFilename(rf))
            .WillOnce(testing::Return(FNameCache{cachedFile, true}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getFilename(rf2))
            .WillOnce(testing::Return(FNameCache{cachedFile2, true}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), fileClosed(cachedFile));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), fileClosed(cachedFile2));
    EXPECT_CALL(*mainInj->create<std::shared_ptr<ReaderM>>(), openBlocking()).Times(2);
    EXPECT_CALL(*mainInj->create<std::shared_ptr<ReaderM>>(), read(10, 200)).WillOnce(testing::Return(fileContents));
    EXPECT_CALL(*mainInj->create<std::shared_ptr<ReaderM>>(), read(100, 200)).WillOnce(testing::Return(fileContents2));
    auto id = static_cast<uint_fast32_t>(t->open(file));
    auto id2 = static_cast<uint_fast32_t>(t->open(file2));
    EXPECT_NE(id, id2);
    EXPECT_EQ(t->read(id, 10, 200), fileContents);
    EXPECT_EQ(t->read(id2, 100, 200), fileContents2);
    t->close(id);
    t->close(id2);
}

TEST_F(FileManagerT, GetSizes){ //NOLINT
    auto t = inj.create<std::shared_ptr<FileManager>>();
    EXPECT_CALL(*inj.create<std::shared_ptr<VkApiM>>(), getRemoteFile(file)).WillOnce(testing::Return(rf));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getFileSize(rf)).WillOnce(testing::Return(1000));
    EXPECT_EQ(t->getFileSize(file), 1000);
}