#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fs/FsUtils.h>
#include <fs/Dir.h>
#include <fs/File.h>
#include <fs/FsException.h>

using vk_music_fs::fs::FsUtils;
using vk_music_fs::fs::DirPtr;
using vk_music_fs::fs::DirWPtr;
using vk_music_fs::fs::Dir;
using vk_music_fs::fs::File;
using vk_music_fs::FileOrDirMeta;
using vk_music_fs::fs::QueryParams;

class FsUtilsT: public ::testing::Test {
public:
    FsUtils _utils;
    DirPtr _rootDir;
    void initFs(){
        _rootDir = std::make_shared<Dir>("/", 1, std::nullopt, DirWPtr{});
        _rootDir->addItem(std::make_shared<Dir>("dir0", 2, std::nullopt, _rootDir));
        auto dir1 = std::make_shared<Dir>("dir1", 3, std::nullopt, _rootDir);
        _rootDir->addItem(dir1);
        auto dir12 = std::make_shared<Dir>("dir12", 4, std::nullopt, dir1);
        auto dir13 = std::make_shared<Dir>("dir13", 5, std::nullopt, dir1);
        dir1->addItem(dir12);
        dir1->addItem(dir13);
        auto file123 = std::make_shared<File>("file123", 6, 1, std::nullopt, dir12);
        dir13->addItem(file123);
        _rootDir->addItem(std::make_shared<Dir>("dir2", 7, std::nullopt, _rootDir));
        auto dir3 = std::make_shared<Dir>("dir3", 8, std::nullopt, _rootDir);
        _rootDir->addItem(dir3);
    }
};

TEST_F(FsUtilsT, FileFound){
    initFs();
    auto path = _utils.findPath(_rootDir, "/dir1/dir13/file123");
    ASSERT_EQ(path.getLast().getName(), "file123");
    path.unlockAll();
}

TEST_F(FsUtilsT, DirFound){
    initFs();
    auto path = _utils.findPath(_rootDir, "/dir1/dir12");
    ASSERT_EQ(path.getLast().getName(), "dir12");
    path.unlockAll();
}

TEST_F(FsUtilsT, NotFound){
    initFs();
    auto path = _utils.findPath(_rootDir, "/dir1/dir13/file1234");
    ASSERT_FALSE(path.isPathMatched());
    path.unlockAll();
}

TEST_F(FsUtilsT, NotFound2Level){
    initFs();
    auto path = _utils.findPath(_rootDir, "/dir1/dir13/file1234", 2);
    ASSERT_TRUE(path.isParentPathMatched());
    ASSERT_EQ((*path.getAll().cbegin()).getName(), "dir13");
    path.unlockAll();
}

TEST_F(FsUtilsT, GetEntriesRoot){
    initFs();
    auto entries = _utils.getEntries(_rootDir, "/", "/");
    auto exp = std::vector<std::string>{"dir0", "dir1", "dir2", "dir3"};
    std::sort(entries.begin(), entries.end());
    ASSERT_EQ(entries, exp);
}

TEST_F(FsUtilsT, GetEntriesNotRoot){
    initFs();
    auto entries = _utils.getEntries(_rootDir, "/dir1/", "/dir1/");
    auto exp = std::vector<std::string>{"dir12", "dir13"};
    std::sort(entries.begin(), entries.end());
    ASSERT_EQ(entries, exp);
}

TEST_F(FsUtilsT, GetEntriesExc){
    initFs();
    try {
        auto entries = _utils.getEntries(_rootDir, "/dir1/gg", "/dir1/gg");
        FAIL();
    } catch (const vk_music_fs::fs::FsException &ex){
    }
}

TEST_F(FsUtilsT, GetMetaFile){
    initFs();
    EXPECT_EQ(_utils.getMeta(_rootDir, "/dir1/dir13/file123").type, FileOrDirMeta::Type::FILE_ENTRY);
    EXPECT_EQ(_utils.getMeta(_rootDir, "/dir1/dir13/file123").time, 1);
}

TEST_F(FsUtilsT, GetMetaDir){
    initFs();
    EXPECT_EQ(_utils.getMeta(_rootDir, "/dir1").type, FileOrDirMeta::Type::DIR_ENTRY);
}

TEST_F(FsUtilsT, GetMetaNotExistant){
    initFs();
    EXPECT_EQ(_utils.getMeta(_rootDir, "/dir1/dir12/file123").type, FileOrDirMeta::Type::NOT_EXISTS);
}

TEST_F(FsUtilsT, ParseQueryTwoNums){
    QueryParams ret{QueryParams::Type::TWO_NUMBERS, 100, 300};
    EXPECT_EQ(_utils.parseQuery("100-300"), ret);
}

TEST_F(FsUtilsT, ParseQueryOneNum){
    QueryParams ret{QueryParams::Type::ONE_NUMBER, 1234, 0};
    EXPECT_EQ(_utils.parseQuery("1234"), ret);
}

TEST_F(FsUtilsT, ParseQueryString){
    QueryParams ret{QueryParams::Type::STRING, 0, 0};
    EXPECT_EQ(_utils.parseQuery("1234h"), ret);
}

TEST_F(FsUtilsT, StripPathPrefix){
    EXPECT_EQ(_utils.stripPathPrefix("/My audios/11", "My audios"), "11");
}