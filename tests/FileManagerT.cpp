#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/di.hpp>
#include <diext/common_di.h>
#include <boost/di/extension/scopes/scoped.hpp>
#include <FileManager.h>
#include <diext/ext_factory.hpp>

using vk_music_fs::ByteVect;
using vk_music_fs::InjPtr;
using vk_music_fs::RemoteFile;
using vk_music_fs::FNameCache;
namespace di = boost::di;

class FileCacheM0{
public:
    template <typename... T>
    FileCacheM0(T&&... args){} //NOLINT
    MOCK_CONST_METHOD1(getFilename, FNameCache(const RemoteFile &f));
    MOCK_CONST_METHOD1(getFileSize, uint_fast32_t(const RemoteFile &file));
    MOCK_CONST_METHOD1(getTagSize, uint_fast32_t(const RemoteFile &file));
    MOCK_CONST_METHOD1(removeSize, uint_fast32_t(const vk_music_fs::RemoteFileId &file));
};

class FileProcessorM0{
public:
    template <typename... T>
    FileProcessorM0(T&&... args){} //NOLINT
    MOCK_CONST_METHOD0(close, void());
    MOCK_CONST_METHOD2(read, ByteVect(uint_fast32_t offset, uint_fast32_t size));
};

class ReaderM0{
public:
    template <typename... T>
    ReaderM0(T&&... args){} //NOLINT
    MOCK_CONST_METHOD2(read, ByteVect(uint_fast32_t offset, uint_fast32_t size));
};

typedef testing::NiceMock<FileCacheM0> FileCacheM;
typedef testing::NiceMock<FileProcessorM0> FileProcessorM;
typedef testing::NiceMock<ReaderM0> ReaderM;

typedef di::extension::iextfactory<
        ReaderM,
        vk_music_fs::CachedFilename,
        vk_music_fs::FileSize
> IReaderFact;

typedef di::extension::iextfactory<FileProcessorM,
        vk_music_fs::Artist,
        vk_music_fs::Title,
        vk_music_fs::Mp3Uri,
        vk_music_fs::TagSize,
        RemoteFile,
        vk_music_fs::CachedFilename
> IProcFact;

class ReaderFact: public IReaderFact{
public:
    template <typename... T>
    ReaderFact(T&&... args){}
    MOCK_CONST_METHOD2(createShared, std::shared_ptr<ReaderM>(
            vk_music_fs::CachedFilename, vk_music_fs::FileSize
    ));
};

class ProcFact: public IProcFact{
public:
    template <typename... T>
    ProcFact(T&&... args){}
    MOCK_CONST_METHOD6(
            createShared,
            std::shared_ptr<FileProcessorM>(
                    vk_music_fs::Artist,
                    vk_music_fs::Title,
                    vk_music_fs::Mp3Uri,
                    vk_music_fs::TagSize,
                    RemoteFile,
                 vk_music_fs::CachedFilename
            )
    );
};

class FileManagerT: public ::testing::Test {
public:
    typedef vk_music_fs::FileManager<FileCacheM, FileProcessorM, ReaderM> FileManager;

    auto_init(inj, (vk_music_fs::makeStorageInj(
            di::bind<IReaderFact>.to<ReaderFact>(),
            di::bind<IProcFact>.to<ProcFact>()
    )));

    std::string file = "/baby.mp3";
    std::string file2 = "/baby2.mp3";
    std::string cachedFile = "/cachedBaby.mp3";
    std::string cachedFile2 = "/cachedBaby2.mp3";
    RemoteFile rf{"http://example.net/", 0, 0, "Justin Bieber", "Baby"};
    RemoteFile rf2{"http://example2.net/", 2, 3, "Justin Bieber", "Baby"};
    ByteVect fileContents{0, 1, 2, 3, 4};
    ByteVect fileContents2{0, 1, 2, 3, 4, 5};

protected:
    void createReaders(){
        auto mock = inj.create<std::shared_ptr<IReaderFact>>();
        EXPECT_CALL(dynamic_cast<ReaderFact&>(*mock), createShared(testing::_, testing::_))
        .WillRepeatedly(testing::Invoke([this] (...) {
            return inj.create<std::shared_ptr<ReaderM>>();
        }));
    }

    void createProcs(){
        auto mock = inj.create<std::shared_ptr<IProcFact>>();
        EXPECT_CALL(
                dynamic_cast<ProcFact&>(*mock),
                createShared(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)
        ).WillRepeatedly(testing::Invoke([this] (...) {
            return inj.create<std::shared_ptr<FileProcessorM>>();
        }));
    }
};

TEST_F(FileManagerT, OpenFileCache){ //NOLINT
    createReaders();
    auto t = inj.create<std::shared_ptr<FileManager>>();
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getFilename(rf))
        .WillOnce(testing::Return(FNameCache{cachedFile, true}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getTagSize(rf)).Times(0);
    t->close(static_cast<uint_fast32_t>(t->open(rf, file)));
}

TEST_F(FileManagerT, OpenFileNoCache){ //NOLINT
    createProcs();
    auto t = inj.create<std::shared_ptr<FileManager>>();
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getFilename(rf))
        .WillOnce(testing::Return(FNameCache{cachedFile, false}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getTagSize(rf));
    t->close(static_cast<uint_fast32_t>(t->open(rf, file)));
}

TEST_F(FileManagerT, OpenFileCacheRead){ //NOLINT
    createReaders();
    auto t = inj.create<std::shared_ptr<FileManager>>();
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getFilename(rf))
            .WillOnce(testing::Return(FNameCache{cachedFile, true}));
    EXPECT_CALL(*inj.create<std::shared_ptr<ReaderM>>(), read(10, 200)).WillOnce(testing::Return(fileContents));
    auto id = static_cast<uint_fast32_t>(t->open(rf, file));
    EXPECT_EQ(t->read(id, 10, 200), fileContents);
    t->close(id);
}

TEST_F(FileManagerT, OpenFileNoCacheRead){ //NOLINT
    createProcs();
    auto t = inj.create<std::shared_ptr<FileManager>>();
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getFilename(rf))
            .WillOnce(testing::Return(FNameCache{cachedFile, false}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileProcessorM>>(), read(10, 200)).WillOnce(testing::Return(fileContents));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileProcessorM>>(), close());
    auto id = static_cast<uint_fast32_t>(t->open(rf, file));
    EXPECT_EQ(t->read(id, 10, 200), fileContents);
    t->close(id);
}

TEST_F(FileManagerT, OpenFileCacheRead2Times){ //NOLINT
    createReaders();
    auto t = inj.create<std::shared_ptr<FileManager>>();
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getFilename(rf))
            .WillOnce(testing::Return(FNameCache{cachedFile, true}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getFilename(rf2))
            .WillOnce(testing::Return(FNameCache{cachedFile2, true}));
    EXPECT_CALL(*inj.create<std::shared_ptr<ReaderM>>(), read(10, 200)).WillOnce(testing::Return(fileContents));
    EXPECT_CALL(*inj.create<std::shared_ptr<ReaderM>>(), read(100, 200)).WillOnce(testing::Return(fileContents2));
    auto id = static_cast<uint_fast32_t>(t->open(rf, file));
    auto id2 = static_cast<uint_fast32_t>(t->open(rf2, file2));
    EXPECT_NE(id, id2);
    EXPECT_EQ(t->read(id, 10, 200), fileContents);
    EXPECT_EQ(t->read(id2, 100, 200), fileContents2);
    t->close(id);
    t->close(id2);
}

TEST_F(FileManagerT, GetSizes){ //NOLINT
    auto t = inj.create<std::shared_ptr<FileManager>>();
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getFileSize(rf)).WillOnce(testing::Return(1000));
    EXPECT_EQ(t->getFileSize(rf, file), 1000);
}

TEST_F(FileManagerT, GetSizesExc){ //NOLINT
    auto t = inj.create<std::shared_ptr<FileManager>>();
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getFileSize(rf)).WillOnce(testing::Invoke(
            [](...) -> uint_fast32_t {
                throw vk_music_fs::net::HttpException("Test error");
            }
    )).WillOnce(testing::Return(500));
    try{
        t->getFileSize(rf, file);
        FAIL();
    } catch (const vk_music_fs::RemoteException &ex){
    }
    EXPECT_EQ(t->getFileSize(rf, file), 500);
}

TEST_F(FileManagerT, OpenFileExc){ //NOLINT
    createReaders();
    auto t = inj.create<std::shared_ptr<FileManager>>();
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getFilename(rf))
            .WillRepeatedly(testing::Invoke([] (...) -> FNameCache {
                throw vk_music_fs::net::HttpException("Test error");
            }));
    try {
        t->close(static_cast<uint_fast32_t>(t->open(rf, file)));
        FAIL();
    } catch (const vk_music_fs::RemoteException &ex){
    }
    try {
        t->close(static_cast<uint_fast32_t>(t->open(rf, file)));
        FAIL();
    } catch (const vk_music_fs::RemoteException &ex){
    }
}

TEST_F(FileManagerT, OpenFileNoCacheReadExc){ //NOLINT
    createProcs();
    auto t = inj.create<std::shared_ptr<FileManager>>();
    EXPECT_CALL(*inj.create<std::shared_ptr<FileCacheM>>(), getFilename(rf))
            .WillOnce(testing::Return(FNameCache{cachedFile, false}));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileProcessorM>>(), read(10, 200)).WillOnce(testing::Invoke(
            [] (...) -> ByteVect{
                throw vk_music_fs::net::HttpException("Test error");
            }));
    EXPECT_CALL(*inj.create<std::shared_ptr<FileProcessorM>>(), close());
    auto id = static_cast<uint_fast32_t>(t->open(rf, file));
    try{
        t->read(id, 10, 200);
        FAIL();
    } catch (const vk_music_fs::RemoteException &ex) {
    }
}
