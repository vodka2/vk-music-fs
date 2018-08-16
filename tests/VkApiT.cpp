#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/di.hpp>
#include <boost/di/extension/scopes/scoped.hpp>
#include <VkApi.h>

namespace di = boost::di;

using vk_music_fs::FileOrDirType;

class QueryMakerM{
public:
    QueryMakerM(){}//NOLINT
    MOCK_CONST_METHOD2(makeSearchQuery, std::string(const std::string&, uint_fast32_t));
};

class VkApiT: public ::testing::Test {
public:

    uint_fast32_t numSearchFiles = 3;

    typedef vk_music_fs::VkApi<QueryMakerM> VkApi;
    auto_init(inj, (di::make_injector(
            di::bind<VkApi>.in(di::extension::scoped),
            di::bind<QueryMakerM>.in(di::extension::scoped),
            di::bind<vk_music_fs::NumSearchFiles>.to(vk_music_fs::NumSearchFiles{numSearchFiles})
            ))
    );
    void initSongNameQuery(){
        EXPECT_CALL(*inj.create<std::shared_ptr<QueryMakerM>>(), makeSearchQuery("SongName", 3))
                .WillOnce(testing::Return(
                        R"(
                        {"response": {"count":3, "items": [
                            {"id": 1, "owner_id": 2, "artist":"Artist1", "title":"Song1", "url":"https:\/\/uri1"},
                            {"id": 2, "owner_id": 3, "artist":"Artist2", "title":"Song2", "url":"https:\/\/uri2"},
                            {"id": -1, "owner_id": 2, "artist":"Artist3", "title":"Song3", "url":"https:\/\/uri3"}
                        ] }}
                        )"
                ));
    }
};

TEST_F(VkApiT, Empty){ //NOLINT
    auto api = inj.create<std::shared_ptr<VkApi>>();
    EXPECT_EQ(api->getEntries("/").size(), 0);
}

TEST_F(VkApiT, CreateDir){ //NOLINT
    auto api = inj.create<std::shared_ptr<VkApi>>();
    initSongNameQuery();
    api->createDir("/SongName");
    std::vector<std::string> expDirs = {"SongName"};
    EXPECT_EQ(api->getEntries("/"), expDirs);
    std::vector<std::string> expFiles = {"Artist1 - Song1.mp3", "Artist2 - Song2.mp3", "Artist3 - Song3.mp3"};
    auto files = api->getEntries("/SongName");
    std::sort(files.begin(), files.end());
    EXPECT_EQ(files, expFiles);
    EXPECT_EQ(api->getRemoteFile("/SongName/Artist2 - Song2.mp3").getOwnerId(), 3);
    EXPECT_EQ(api->getRemoteFile("/SongName/Artist3 - Song3.mp3").getUri(), "https://uri3");
}

TEST_F(VkApiT, GetType){ //NOLINT
    auto api = inj.create<std::shared_ptr<VkApi>>();
    EXPECT_EQ(api->getType("/"), FileOrDirType::DIR_ENTRY);
    initSongNameQuery();
    api->createDir("/SongName");
    EXPECT_EQ(api->getType("/SongName"), FileOrDirType::DIR_ENTRY);
    EXPECT_EQ(api->getType("/SongName/Artist2 - Song2.mp3"), FileOrDirType::FILE_ENTRY);
    EXPECT_EQ(api->getType("/song"), FileOrDirType::NOT_EXISTS);
}

TEST_F(VkApiT, CreateDummyDir){ //NOLINT
    auto api = inj.create<std::shared_ptr<VkApi>>();
    initSongNameQuery();
    api->createDummyDir("/New Folder");
    EXPECT_EQ(api->getEntries("/New Folder").size(), 0);
    api->renameDummyDir("/New Folder", "/SongName");
    std::vector<std::string> expFiles = {"Artist1 - Song1.mp3", "Artist2 - Song2.mp3", "Artist3 - Song3.mp3"};
    auto files = api->getEntries("/SongName");
    std::sort(files.begin(), files.end());
    EXPECT_EQ(files, expFiles);
}