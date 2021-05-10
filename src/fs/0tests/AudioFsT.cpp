#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/di.hpp>
#include <boost/di/extension/scopes/scoped.hpp>
#include <fs/AudioFs.h>
#include <mp3core/RemoteException.h>
#include <diext/common_di.h>
#include <fs/FsUtils.h>
#include <fs/FileObtainer.h>
#include <fs/FsSettings.h>
#include <fs/ctrl/DummyDirWrapper.h>
#include <fs/ctrl/RemoteFileWrapper.h>
#include <fs/ctrl/MyAudiosCtrl.h>
#include <fs/ctrl/SearchSongNameCtrl.h>
#include <fs/ctrl/SearchSongNameArtistHelper.h>
#include <fs/ctrl/SearchSongNameSongHelper.h>
#include <fs/ctrl/RootCtrl.h>
#include <fs/ctrl/SingleDirCtrl.h>
#include <fs/actions/act.h>
#include <fs/AsyncFsManager.h>
#include "data/FileManagerM.h"
#include "fs/0tests/data/PhotoManagerM.h"
#include "data/FileCacheM.h"
#include "data/PhotoCacheM.h"
#include "data/RealFsM.h"
#include "data/ThreadPoolM.h"

namespace di = boost::di;

using vk_music_fs::FileOrDirMeta;

class QueryMakerM0{
public:
    template <typename... T>
    QueryMakerM0(T&&... args){}//NOLINT
    MOCK_CONST_METHOD3(makeSearchQuery, std::string(const std::string&, uint_fast32_t, uint_fast32_t));
    MOCK_CONST_METHOD3(makeArtistSearchQuery, std::string(const std::string&, uint_fast32_t, uint_fast32_t));
    MOCK_CONST_METHOD2(makeMyAudiosQuery, std::string(uint_fast32_t, uint_fast32_t));
    MOCK_CONST_METHOD3(makeMyPlaylistsQuery, std::string(uint_fast32_t, uint_fast32_t, uint_fast32_t));
    MOCK_CONST_METHOD5(getPlaylistAudios, std::string(std::string, int_fast32_t, uint_fast32_t, uint_fast32_t, uint_fast32_t));
    MOCK_CONST_METHOD2(addToMyAudios, std::string(int_fast32_t, uint_fast32_t));
    MOCK_CONST_METHOD2(deleteFromMyAudios, std::string(int_fast32_t, uint_fast32_t));
    MOCK_CONST_METHOD2(deletePlaylist, std::string(int_fast32_t, uint_fast32_t));
    MOCK_CONST_METHOD3(searchSimilar, std::string(const std::string &, uint_fast32_t, uint_fast32_t));
    MOCK_CONST_METHOD0(getUserId, std::string());
};

typedef testing::NiceMock<QueryMakerM0> QueryMakerM;


class AudioFsT: public ::testing::Test {
public:
    uint_fast32_t numSearchFiles = 3;

    typedef vk_music_fs::AudioFs<
            vk_music_fs::fs::CtrlTuple<
                    vk_music_fs::fs::FsUtils, vk_music_fs::fs::FileObtainer<QueryMakerM>, FileManagerM, PhotoManagerM,
                    vk_music_fs::fs::AsyncFsManager<vk_music_fs::fs::FsUtils, FileCacheM, PhotoCacheM, RealFsM, ThreadPoolM>
                    >
    > AudioFs;

    auto makeInj(bool createDummyDirs){
        return vk_music_fs::makeStorageInj(
                di::bind<vk_music_fs::Mp3Extension>.to(vk_music_fs::Mp3Extension{".mp3"}),
                di::bind<vk_music_fs::NumSearchFiles>.to(vk_music_fs::NumSearchFiles{numSearchFiles}),
                di::bind<vk_music_fs::CreateDummyDirs>.to(vk_music_fs::CreateDummyDirs{createDummyDirs}),
                di::bind<vk_music_fs::fs::UseAsyncNotifier>.to(vk_music_fs::fs::UseAsyncNotifier{false}),
                di::bind<vk_music_fs::fs::PhotoName>.to(vk_music_fs::fs::PhotoName{"photo.jpg"}),
                di::bind<vk_music_fs::fs::PathToFs>.to(vk_music_fs::fs::PathToFs{"/"})
        );
    }

    di::injector<std::shared_ptr<AudioFs>, std::shared_ptr<QueryMakerM>, std::shared_ptr<FileManagerM>> inj;
    di::injector<std::shared_ptr<AudioFs>, std::shared_ptr<QueryMakerM>, std::shared_ptr<FileManagerM>> dummyInj;
    AudioFsT(): inj(makeInj(false)), dummyInj(makeInj(true)){
        setCreateDummyDirs(false);
    }

    auto vectToSet(const std::vector<std::string> &vect) {
        return std::set<std::string>{vect.cbegin(), vect.cend()};
    }

    std::shared_ptr<QueryMakerM> queryMakerM;
    std::shared_ptr<FileManagerM> fileManagerM;
    void setCreateDummyDirs(bool createDummyDirs){
        if(createDummyDirs){
            queryMakerM = dummyInj.create<std::shared_ptr<QueryMakerM>>();
            fileManagerM = dummyInj.create<std::shared_ptr<FileManagerM>>();
        } else {
            queryMakerM = inj.create<std::shared_ptr<QueryMakerM>>();
            fileManagerM = inj.create<std::shared_ptr<FileManagerM>>();
        }
    }

    void initArtistQuery(){
        ON_CALL(*queryMakerM, makeArtistSearchQuery("Artist", 0, 3)).WillByDefault(testing::Return(
                R"(
                        {"response": {"count":3, "items": [
                            {"id": 1, "owner_id": 2, "artist":"Artist1", "title":"Song1", "url":"https:\/\/uri1"},
                            {"id": 2, "owner_id": 3, "artist":"Artist2", "title":"Song2", "url":"https:\/\/uri2"},
                            {"id": -1, "owner_id": 2, "artist":"Artist3", "title":"Song3", "url":"https:\/\/uri3"}
                        ] }}
                        )"
        ));
    }

    void initArtistSecondQuery(){
        ON_CALL(*queryMakerM, makeArtistSearchQuery("Artist", 3, 1)).WillByDefault(testing::Return(
                R"(
                        {"response": {"count":1, "items": [
                            {"id": -1, "owner_id": 5, "artist":"Artist4", "title":"Song4", "url":"https:\/\/uri4"}
                        ] }}
                        )"
        ));
    }

    void initMyAudiosQuery(){
        ON_CALL(*queryMakerM, makeMyAudiosQuery(0, 3)).WillByDefault(testing::Return(
                R"(
                        {"response": {"count":3, "items": [
                            {"id": 1, "owner_id": 2, "artist":"Artist1", "title":"Song1", "url":"https:\/\/uri1"},
                            {"id": 2, "owner_id": 3, "artist":"Artist2", "title":"Song2", "url":"https:\/\/uri2"},
                            {"id": -1, "owner_id": 2, "artist":"Artist3", "title":"Song3", "url":"https:\/\/uri3"}
                        ] }}
                        )"
        ));
    }

    void initMyAudiosQuery2(){
        ON_CALL(*queryMakerM, makeMyAudiosQuery(0, 3)).WillByDefault(testing::Return(
                R"(
                        {"response": {"count":3, "items": [
                            {"id": 1, "owner_id": 2, "artist":"Artist1", "title":"Song1", "url":"https:\/\/uri1"},
                            {"id": 2, "owner_id": 3, "artist":"Artist2", "title":"Song2", "url":"https:\/\/uri2"},
                            {"id": -1, "owner_id": 6, "artist":"Artist5", "title":"Song5", "url":"https:\/\/uri3"}
                        ] }}
                        )"
        ));
    }

    void initMyAudiosIntervalQuery(){
        ON_CALL(*queryMakerM, makeMyAudiosQuery(1, 2)).WillByDefault(testing::Return(
                R"(
                        {"response": {"count":2, "items": [
                            {"id": 2, "owner_id": 3, "artist":"Artist2", "title":"Song2", "url":"https:\/\/uri2"},
                            {"id": -1, "owner_id": 2, "artist":"Artist3", "title":"Song3", "url":"https:\/\/uri3"}
                        ] }}
                        )"
        ));
    }

    void initMyAudiosAfterIntervalQuery(){
        ON_CALL(*queryMakerM, makeMyAudiosQuery(3, 1)).WillByDefault(testing::Return(
                R"(
                        {"response": {"count":2, "items": [
                            {"id": -1, "owner_id": 2, "artist":"Artist4", "title":"Song4", "url":"https:\/\/uri3"}
                        ] }}
                        )"
        ));
    }

    void initMyAudiosDeleteFileQuery(){
        ON_CALL(*queryMakerM, deleteFromMyAudios(3, 2)).WillByDefault(testing::Return(
                R"(
                        {"response": {}}
                        )"
        ));
    }

    void initSongNameQuery(){
        ON_CALL(*queryMakerM, makeSearchQuery("SongName", 0, 3)).WillByDefault(testing::Return(
                        R"(
                        {"response": {"count":3, "items": [
                            {"id": 1, "owner_id": 2, "artist":"Artist1", "title":"Song1", "url":"https:\/\/uri1"},
                            {"id": 2, "owner_id": 3, "artist":"Artist2", "title":"Song2", "url":"https:\/\/uri2"},
                            {"id": -1, "owner_id": 2, "artist":"Artist3", "title":"Song3", "url":"https:\/\/uri3"}
                        ] }}
                        )"
                ));
    }

    void initSongNameQueryVkErr(){
        ON_CALL(*queryMakerM, makeSearchQuery("SongName", 0, 3)).WillByDefault(testing::Return(
                R"(
                        {"error":{"error_code":134, "error_msg":"Some message"}}
                        )"
        ));
    }

    void initSongNameQueryHttpErr(){
        ON_CALL(*queryMakerM, makeSearchQuery("SongName", 0, 3)).WillByDefault(testing::Invoke(
                [] (...) -> std::string{
                    throw vk_music_fs::RemoteException("Test error");
                })
        );
    }

    void initSmallerSongNameQuery(){
        ON_CALL(*queryMakerM, makeSearchQuery("SongName", 0, 2)).WillByDefault(testing::Return(
                R"(
                        {"response": {"count":3, "items": [
                            {"id": 1, "owner_id": 2, "artist":"Artist1", "title":"Song1", "url":"https:\/\/uri1"},
                            {"id": 2, "owner_id": 3, "artist":"Artist2", "title":"Song2", "url":"https:\/\/uri2"}
                        ] }}
                        )"
        ));
    }
    void initSongName2Query(){
        ON_CALL(*queryMakerM, makeSearchQuery("SongName2", 0, 3))
                .WillByDefault(testing::Return(
                        R"(
                        {"response": {"count":3, "items": [
                            {"id": 1, "owner_id": 22, "artist":"Artist21", "title":"Song21", "url":"https:\/\/uri21"},
                            {"id": 2, "owner_id": 23, "artist":"Artist22", "title":"Song22", "url":"https:\/\/uri22"},
                            {"id": -1, "owner_id": 22, "artist":"Artist23", "title":"Song23", "url":"https:\/\/uri23"}
                        ] }}
                        )"
                ));
    }
    void initSongNameSecondQuery(){
        ON_CALL(*queryMakerM, makeSearchQuery("SongName", 3, 1))
                .WillByDefault(testing::Return(
                        R"(
                        {"response": {"count":1, "items": [
                            {"id": 1, "owner_id": 5, "artist":"Artist4", "title":"Song4", "url":"https:\/\/uri4"}
                        ] }}
                        )"
                ));
    }
    void initSongNameNameQuery(){
        ON_CALL(*queryMakerM, makeSearchQuery("SongName Name", 0, 3)).WillByDefault(testing::Return(
                R"(
                        {"response": {"count":1, "items": [
                            {"id": 9, "owner_id": 2, "artist":"Artist1", "title":"SongName Name", "url":"https:\/\/uri5"}
                        ] }}
                        )"
        ));
    }

    void initRenameFileQuery(){
        ON_CALL(*queryMakerM, addToMyAudios(3, 2)).WillByDefault(testing::Return(
                R"(
                        {"response": {}}
                        )"
        ));
    }

    void initGetUserIdQuery(){
        ON_CALL(*queryMakerM, getUserId()).WillByDefault(testing::Return(
                R"(
                        {"response": [{
                           "id" : 456
                        }]}
                        )"
        ));
    }

    void initMyPlaylistsQuery(){
        ON_CALL(*queryMakerM, makeMyPlaylistsQuery(456, 0, 3)).WillByDefault(testing::Return(
                R"(
                        {"response": {
                           "count": 3,
                           "items": [
                               {"owner_id": 5, "id": 7, "access_key": "8", "title": "Playlist name"},
                               {"owner_id": -5, "id": 11, "access_key": "7", "title": "Playlist name 2"},
                               {"owner_id": -9, "id": 1, "access_key": "17", "title": "Playlist name 3", "album_type": "main_only"}
                           ]
                        }}
                        )"
        ));
    }

    void initMyPlaylistsAudiosQuery(){
        ON_CALL(*queryMakerM, getPlaylistAudios("7", -5, 11, 0, 2)).WillByDefault(testing::Return(
                R"(
                        {"response": {
                           "count": 2,
                           "items": [
                               {"id": 1, "owner_id": 2, "artist":"Artist1", "title":"Song1", "url":"https:\/\/uri1"},
                               {"id": 2, "owner_id": 3, "artist":"Artist2", "title":"Song2", "url":"https:\/\/uri2"}
                           ]
                        }}
                        )"
        ));
    }

    void initMyPlaylistsAlbumAudiosQuery(){
        ON_CALL(*queryMakerM, getPlaylistAudios("17", -9, 1, 0, 2)).WillByDefault(testing::Return(
                R"(
                        {"response": {
                           "count": 2,
                           "items": [
                               {"id": 1, "owner_id": 2, "artist":"Artist1", "title":"Song1", "url":"https:\/\/uri1"},
                               {"id": 2, "owner_id": 3, "artist":"Artist2", "title":"Song2", "url":"https:\/\/uri2"}
                           ]
                        }}
                        )"
        ));
    }

    void initSimilarQuery(){
        ON_CALL(*queryMakerM, searchSimilar("2_1", 0, 3)).WillByDefault(testing::Return(
                R"(
                        {"response": {
                           "count": 2,
                           "items": [
                               {"id": 1, "owner_id": 2, "artist":"Artist1", "title":"Song1", "url":"https:\/\/uri1"},
                               {"id": 2, "owner_id": 3, "artist":"Artist2", "title":"Song2", "url":"https:\/\/uri2"}
                           ]
                        }}
                        )"
        ));
    }
};

TEST_F(AudioFsT, Empty){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    EXPECT_EQ(api->getEntries("/").size(), 4);
    EXPECT_EQ(api->getEntries("/Search").size(), 0);
    EXPECT_EQ(api->getEntries("/Search by artist").size(), 0);
    EXPECT_EQ(api->getEntries("/My playlists").size(), 0);
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
    EXPECT_CALL(*fileManagerM, open(testing::_, "/Search/SongName/Artist3 - Song3.mp3")).WillOnce(
            testing::WithArgs<0>(testing::Invoke([](RemoteFile arg){
                EXPECT_EQ(arg.getUri(), "https://uri3");
                return 77;
            })));
    api->open("/Search/SongName/Artist3 - Song3.mp3");
}

TEST_F(AudioFsT, GetType){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    EXPECT_EQ(api->getMeta("/").type, FileOrDirMeta::Type::DIR_ENTRY);
    EXPECT_EQ(api->getMeta("/Search").type, FileOrDirMeta::Type::DIR_ENTRY);
    initSongNameQuery();
    api->createDir("/Search/SongName");
    EXPECT_EQ(api->getMeta("/Search/SongName").type, FileOrDirMeta::Type::DIR_ENTRY);
    EXPECT_EQ(api->getMeta("/Search/SongName/Artist2 - Song2.mp3").type, FileOrDirMeta::Type::FILE_ENTRY);
    EXPECT_EQ(api->getMeta("/Search/song").type, FileOrDirMeta::Type::NOT_EXISTS);
}

TEST_F(AudioFsT, CreateDirVkErr){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initSongNameQueryVkErr();
    try {
        api->createDir("/Search/SongName");
        FAIL();
    } catch (const vk_music_fs::fs::FsException &ex){
    }
}

TEST_F(AudioFsT, CreateDirHttpErr){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initSongNameQueryHttpErr();
    try {
        api->createDir("/Search/SongName");
        FAIL();
    } catch (const vk_music_fs::fs::FsException &ex){
    }
}

TEST_F(AudioFsT, RenameFile){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initSongNameQuery();
    initRenameFileQuery();
    api->createDir("/Search/SongName");
    api->rename("/Search/SongName/Artist2 - Song2.mp3", "/Search/SongName/Artist2 - Song2_a.mp3");
    std::vector<std::string> expData = {"Artist1 - Song1.mp3", "Artist2 - Song2_a.mp3", "Artist3 - Song3.mp3"};
    auto data = api->getEntries("/Search/SongName");
    std::sort(data.begin(), data.end());
    EXPECT_EQ(data, expData);
}

TEST_F(AudioFsT, CreateDummyDir){ //NOLINT
    setCreateDummyDirs(true);
    auto api = dummyInj.create<std::shared_ptr<AudioFs>>();
    initSongNameQuery();
    api->createDir("/Search/New Folder");
    EXPECT_EQ(api->getEntries("/Search/New Folder").size(), 0);
    api->rename("/Search/New Folder", "/Search/SongName");
    std::vector<std::string> expFiles = {"Artist1 - Song1.mp3", "Artist2 - Song2.mp3", "Artist3 - Song3.mp3"};
    auto files = api->getEntries("/Search/SongName");
    std::sort(files.begin(), files.end());
    EXPECT_EQ(files, expFiles);
}

TEST_F(AudioFsT, CreateMoreDirOneNum){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initSongNameQuery();
    initSmallerSongNameQuery();
    api->createDir("/Search/SongName");
    api->createDir("/Search/SongName/2");
    std::vector<std::string> expData = {"2", "Artist1 - Song1.mp3", "Artist2 - Song2.mp3"};
    auto dirs = api->getEntries("/Search/SongName");
    std::sort(dirs.begin(), dirs.end());
    EXPECT_EQ(dirs, expData);
}

TEST_F(AudioFsT, CreateMoreDirTwoNum){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initSongNameQuery();
    initSongNameSecondQuery();
    api->createDir("/Search/SongName");
    api->createDir("/Search/SongName/3-1");
    std::vector<std::string> expData = {"3-1", "Artist4 - Song4.mp3"};
    auto data = api->getEntries("/Search/SongName");
    std::sort(data.begin(), data.end());
    EXPECT_EQ(data, expData);
}

TEST_F(AudioFsT, CreateMoreDirNested){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initSongNameQuery();
    initSongNameNameQuery();
    api->createDir("/Search/SongName");
    api->createDir("/Search/SongName/Name");
    std::vector<std::string> expData = {"Artist1 - Song1.mp3", "Artist2 - Song2.mp3", "Artist3 - Song3.mp3", "Name"};
    auto data = api->getEntries("/Search/SongName");
    std::sort(data.begin(), data.end());
    EXPECT_EQ(data, expData);
    std::vector<std::string> expFiles = {"Artist1 - SongName Name.mp3"};
    auto files = api->getEntries("/Search/SongName/Name");
    EXPECT_EQ(files, expFiles);
}

TEST_F(AudioFsT, CreateMoreDirNestedThenDelete){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initSongNameQuery();
    initSongNameNameQuery();
    api->createDir("/Search/SongName");
    api->createDir("/Search/SongName/3");
    api->createDir("/Search/SongName/Name");
    api->deleteDir("/Search/SongName/3");
    std::vector<std::string> expData = {"Artist1 - Song1.mp3", "Artist2 - Song2.mp3", "Artist3 - Song3.mp3", "Name"};
    auto data = api->getEntries("/Search/SongName");
    std::sort(data.begin(), data.end());
    EXPECT_EQ(data, expData);
    std::vector<std::string> expFiles = {"Artist1 - SongName Name.mp3"};
    auto files = api->getEntries("/Search/SongName/Name");
    EXPECT_EQ(files, expFiles);
}

TEST_F(AudioFsT, DeleteDir){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initSongNameQuery();
    initSongName2Query();
    api->createDir("/Search/SongName");
    api->createDir("/Search/SongName2");
    api->deleteDir("/Search/SongName2");
    EXPECT_EQ(api->getEntries("/Search").size(), 1);
    api->deleteDir("/Search/SongName");
    EXPECT_EQ(api->getEntries("/Search").size(), 0);
}

TEST_F(AudioFsT, DeleteFile){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initSongNameQuery();
    api->createDir("/Search/SongName");
    api->deleteFile("/Search/SongName/Artist2 - Song2.mp3");
    std::vector<std::string> expFiles = {"Artist1 - Song1.mp3", "Artist3 - Song3.mp3"};
    auto files = api->getEntries("/Search/SongName");
    std::sort(files.begin(), files.end());
    EXPECT_EQ(files, expFiles);
}

TEST_F(AudioFsT, CreateMyAudiosDirOneNum){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initMyAudiosQuery();
    api->createDir("/My audios/3");
    std::vector<std::string> expFiles = {"3", "Artist1 - Song1.mp3", "Artist2 - Song2.mp3", "Artist3 - Song3.mp3"};
    auto files = api->getEntries("/My audios");
    std::sort(files.begin(), files.end());
    EXPECT_EQ(files, expFiles);
}

TEST_F(AudioFsT, DeleteMyAudiosFile){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initMyAudiosQuery();
    initMyAudiosDeleteFileQuery();
    api->createDir("/My audios/3");
    api->deleteFile("/My audios/Artist2 - Song2.mp3");
    auto files = api->getEntries("/My audios");
    std::vector<std::string> expFiles = {"3", "Artist1 - Song1.mp3", "Artist3 - Song3.mp3"};
    std::sort(files.begin(), files.end());
    EXPECT_EQ(files, expFiles);
}

TEST_F(AudioFsT, CreateMyAudiosDirOneNumRefresh){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initMyAudiosQuery();
    initMyAudiosQuery2();
    api->createDir("/My audios/3");
    api->createDir("/My audios/r4");
    std::vector<std::string> expFiles = {"3", "Artist1 - Song1.mp3", "Artist2 - Song2.mp3", "Artist5 - Song5.mp3",  "r4"};
    auto files = api->getEntries("/My audios");
    std::sort(files.begin(), files.end());
    EXPECT_EQ(files, expFiles);
}

TEST_F(AudioFsT, CreateMyAudiosDirTwoNum){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initMyAudiosIntervalQuery();
    api->createDir("/My audios/1-2");
    std::vector<std::string> expFiles = {"1-2", "Artist2 - Song2.mp3", "Artist3 - Song3.mp3"};
    auto files = api->getEntries("/My audios");
    std::sort(files.begin(), files.end());
    EXPECT_EQ(files, expFiles);
}

TEST_F(AudioFsT, CreateMyAudiosDirTwoNumThenOneNum){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initMyAudiosIntervalQuery();
    initMyAudiosAfterIntervalQuery();
    api->createDir("/My audios/1-2");
    api->createDir("/My audios/3");
    std::vector<std::string> expFiles = {"3", "Artist2 - Song2.mp3", "Artist3 - Song3.mp3", "Artist4 - Song4.mp3"};
    auto files = api->getEntries("/My audios");
    std::sort(files.begin(), files.end());
    EXPECT_EQ(files, expFiles);
}

TEST_F(AudioFsT, CreateMyAudiosDirTwoNumThenOneNumLess){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initMyAudiosIntervalQuery();
    initMyAudiosAfterIntervalQuery();
    api->createDir("/My audios/1-2");
    api->createDir("/My audios/1");
    std::vector<std::string> expFiles = {"1", "Artist2 - Song2.mp3"};
    auto files = api->getEntries("/My audios");
    std::sort(files.begin(), files.end());
    EXPECT_EQ(files, expFiles);
}


TEST_F(AudioFsT, CreateArtistSearchDir){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initArtistQuery();
    api->createDir("/Search by artist/Artist");
    std::vector<std::string> expDirs = {"Artist"};
    EXPECT_EQ(api->getEntries("/Search by artist"), expDirs);
    std::vector<std::string> expFiles = {"Artist1 - Song1.mp3", "Artist2 - Song2.mp3", "Artist3 - Song3.mp3"};
    auto files = api->getEntries("/Search by artist/Artist");
    std::sort(files.begin(), files.end());
    EXPECT_EQ(files, expFiles);
    EXPECT_CALL(*fileManagerM, open(testing::_, "/Search by artist/Artist/Artist3 - Song3.mp3")).WillOnce(
            testing::WithArgs<0>(testing::Invoke([](RemoteFile arg){
                EXPECT_EQ(arg.getUri(), "https://uri3");
                return 77;
            })));
    api->open("/Search by artist/Artist/Artist3 - Song3.mp3");
}

TEST_F(AudioFsT, CreateArtistSearchMoreDirOneNum){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initArtistQuery();
    initArtistSecondQuery();
    api->createDir("/Search by artist/Artist");
    api->createDir("/Search by artist/Artist/4");
    std::vector<std::string> expData = {
            "4", "Artist1 - Song1.mp3", "Artist2 - Song2.mp3", "Artist3 - Song3.mp3", "Artist4 - Song4.mp3"
    };
    auto dirs = api->getEntries("/Search by artist/Artist");
    std::sort(dirs.begin(), dirs.end());
    EXPECT_EQ(dirs, expData);
}

TEST_F(AudioFsT, CreatePlaylists){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initGetUserIdQuery();
    initMyPlaylistsQuery();
    api->createDir("/My playlists/3");
    std::vector<std::string> expData = {
            "3", "Playlist name", "Playlist name 2", "Playlist name 3"
    };
    auto dirs = api->getEntries("/My playlists");
    std::sort(dirs.begin(), dirs.end());
    EXPECT_EQ(dirs, expData);
}

TEST_F(AudioFsT, LoadPlaylistAudios){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initGetUserIdQuery();
    initMyPlaylistsQuery();
    initMyPlaylistsAudiosQuery();
    api->createDir("/My playlists/3");
    api->createDir("/My playlists/Playlist name 2/2");
    auto dirs = api->getEntries("/My playlists/Playlist name 2");
    std::vector<std::string> expData = {
            "2", "Artist1 - Song1.mp3", "Artist2 - Song2.mp3"
    };
    std::sort(dirs.begin(), dirs.end());
    EXPECT_EQ(dirs, expData);
}

TEST_F(AudioFsT, LoadAlbumAudios){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initGetUserIdQuery();
    initMyPlaylistsQuery();
    initMyPlaylistsAlbumAudiosQuery();
    api->createDir("/My playlists/3");
    api->createDir("/My playlists/Playlist name 3/2");
    auto dirs = api->getEntries("/My playlists/Playlist name 3");
    std::vector<std::string> expData = {
            "2", "Artist1 - Song1.mp3", "Artist2 - Song2.mp3"
    };
    std::sort(dirs.begin(), dirs.end());
    EXPECT_EQ(dirs, expData);
    EXPECT_CALL(*fileManagerM,
            open(testing::_, "/My playlists/Playlist name 3/Artist2 - Song2.mp3")).WillOnce(
            testing::WithArgs<0>(testing::Invoke([](RemoteFile arg){
                EXPECT_EQ(*arg.getAlbumName(), "Playlist name 3");
                return 77;
            })));
    api->open("/My playlists/Playlist name 3/Artist2 - Song2.mp3");
}

TEST_F(AudioFsT, CreateMyAudiosSimilarRefresh){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initMyAudiosQuery();
    initSimilarQuery();
    api->createDir("/My audios/3");
    api->rename("/My audios/Artist1 - Song1.mp3", "/My audios/Artist1 - Song1_s.mp3");
    api->createDir("/My audios/r");
    std::set<std::string> expFiles = {
            "3", "Artist1 - Song1.mp3", "Artist2 - Song2.mp3",
            "Artist3 - Song3.mp3", "Artist1 - Song1", "r"
    };
    EXPECT_EQ(expFiles, vectToSet(api->getEntries("/My audios")));
}

TEST_F(AudioFsT, CreateMyAudiosSimilarCounter){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    initMyAudiosQuery();
    initSimilarQuery();
    api->createDir("/My audios/3");
    api->rename("/My audios/Artist1 - Song1.mp3", "/My audios/Artist1 - Song1_s.mp3");
    api->createDir("/My audios/0-3");
    std::set<std::string> expFiles = {
            "0-3", "Artist1 - Song1.mp3", "Artist2 - Song2.mp3",
            "Artist3 - Song3.mp3", "Artist1 - Song1"
    };
    EXPECT_EQ(expFiles, vectToSet(api->getEntries("/My audios")));
}