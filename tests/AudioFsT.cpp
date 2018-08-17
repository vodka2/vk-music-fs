#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/di.hpp>
#include <boost/di/extension/scopes/scoped.hpp>
#include <AudioFs.h>

namespace di = boost::di;

using vk_music_fs::FileOrDirType;

class QueryMakerM{
public:
    QueryMakerM(){}//NOLINT
    MOCK_CONST_METHOD2(makeSearchQuery, std::string(const std::string&, uint_fast32_t));
};

class AudioFsT: public ::testing::Test {
public:

    uint_fast32_t numSearchFiles = 3;

    typedef vk_music_fs::AudioFs<QueryMakerM> AudioFs;
    auto_init(inj, (di::make_injector(
            di::bind<AudioFs>.in(di::extension::scoped),
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

TEST_F(AudioFsT, Empty){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    EXPECT_EQ(api->getEntries("/").size(), 1);
    EXPECT_EQ(api->getEntries("/Search").size(), 0);
}

TEST_F(AudioFsT, CreateDir){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initSongNameQuery();
    api->createDir("/Search/SongName");
    std::vector<std::string> expDirs = {"SongName"};
    EXPECT_EQ(api->getEntries("/Search"), expDirs);
    std::vector<std::string> expFiles = {"Artist1 - Song1.mp3", "Artist2 - Song2.mp3", "Artist3 - Song3.mp3"};
    auto files = api->getEntries("/Search/SongName");
    std::sort(files.begin(), files.end());
    EXPECT_EQ(files, expFiles);
    EXPECT_EQ(api->getRemoteFile("/Search/SongName/Artist2 - Song2.mp3").getOwnerId(), 3);
    EXPECT_EQ(api->getRemoteFile("/Search/SongName/Artist3 - Song3.mp3").getUri(), "https://uri3");
}

TEST_F(AudioFsT, GetType){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    EXPECT_EQ(api->getType("/"), FileOrDirType::DIR_ENTRY);
    EXPECT_EQ(api->getType("/Search"), FileOrDirType::DIR_ENTRY);
    initSongNameQuery();
    api->createDir("/Search/SongName");
    EXPECT_EQ(api->getType("/Search/SongName"), FileOrDirType::DIR_ENTRY);
    EXPECT_EQ(api->getType("/Search/SongName/Artist2 - Song2.mp3"), FileOrDirType::FILE_ENTRY);
    EXPECT_EQ(api->getType("/Search/song"), FileOrDirType::NOT_EXISTS);
}

TEST_F(AudioFsT, CreateDummyDir){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initSongNameQuery();
    api->createDummyDir("/Search/New Folder");
    EXPECT_EQ(api->getEntries("/Search/New Folder").size(), 0);
    api->renameDummyDir("/Search/New Folder", "/Search/SongName");
    std::vector<std::string> expFiles = {"Artist1 - Song1.mp3", "Artist2 - Song2.mp3", "Artist3 - Song3.mp3"};
    auto files = api->getEntries("/Search/SongName");
    std::sort(files.begin(), files.end());
    EXPECT_EQ(files, expFiles);
}