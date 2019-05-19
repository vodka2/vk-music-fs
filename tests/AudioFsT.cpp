#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/di.hpp>
#include <boost/di/extension/scopes/scoped.hpp>
#include <fs/AudioFs.h>
#include <net/HttpException.h>
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
#include "data/FileManagerM.h"

namespace di = boost::di;

using vk_music_fs::FileOrDirMeta;

class QueryMakerM0{
public:
    template <typename... T>
    QueryMakerM0(T&&... args){}//NOLINT
    MOCK_CONST_METHOD3(makeSearchQuery, std::string(const std::string&, uint_fast32_t, uint_fast32_t));
    MOCK_CONST_METHOD3(makeArtistSearchQuery, std::string(const std::string&, uint_fast32_t, uint_fast32_t));
    MOCK_CONST_METHOD2(makeMyAudiosQuery, std::string(uint_fast32_t, uint_fast32_t));
    MOCK_CONST_METHOD2(addToMyAudios, std::string(int_fast32_t, uint_fast32_t));
    MOCK_CONST_METHOD2(deleteFromMyAudios, std::string(int_fast32_t, uint_fast32_t));
};

typedef testing::NiceMock<QueryMakerM0> QueryMakerM;

class AudioFsT: public ::testing::Test {
public:
    uint_fast32_t numSearchFiles = 3;

    typedef vk_music_fs::fs::RefreshAct<vk_music_fs::fs::FsUtils> RefreshActD;
    typedef vk_music_fs::fs::NumberAct<vk_music_fs::fs::FsUtils> NumberActD;
    typedef vk_music_fs::fs::ActTuple<vk_music_fs::fs::FsUtils> ActTupleD;

    typedef vk_music_fs::fs::FileObtainer<QueryMakerM> FileObtainerD;
    typedef vk_music_fs::fs::CtrlTuple<vk_music_fs::fs::FsUtils, FileObtainerD, FileManagerM> CtrlTupleD;
    typedef vk_music_fs::fs::MyAudiosCtrl<vk_music_fs::fs::FsUtils, FileObtainerD> MyAudiosCtrlD;
    typedef vk_music_fs::fs::SingleDirCtrl<MyAudiosCtrlD, vk_music_fs::fs::FsUtils> MyAudiosSingleDirD;
    typedef vk_music_fs::fs::RemoteFileWrapper<MyAudiosSingleDirD, vk_music_fs::fs::FsUtils, FileManagerM> MyAudiosRemoteFileD;
    typedef vk_music_fs::fs::DummyDirWrapper<MyAudiosRemoteFileD, vk_music_fs::fs::FsUtils> MyAudiosDummyDirD;

    typedef vk_music_fs::fs::SearchSongNameCtrl<vk_music_fs::fs::FsUtils, FileObtainerD, vk_music_fs::fs::SearchSongNameArtistHelper> SearchSongNameCtrlD1;
    typedef vk_music_fs::fs::SingleDirCtrl<SearchSongNameCtrlD1, vk_music_fs::fs::FsUtils> SearchSongNameSingleDirD1;
    typedef vk_music_fs::fs::RemoteFileWrapper<SearchSongNameSingleDirD1, vk_music_fs::fs::FsUtils, FileManagerM> SearchSongNameRemoteFileD1;
    typedef vk_music_fs::fs::DummyDirWrapper<SearchSongNameRemoteFileD1, vk_music_fs::fs::FsUtils> SearchSongNameDummyDirD1;

    typedef vk_music_fs::fs::SearchSongNameCtrl<vk_music_fs::fs::FsUtils, FileObtainerD, vk_music_fs::fs::SearchSongNameSongHelper> SearchSongNameCtrlD2;
    typedef vk_music_fs::fs::SingleDirCtrl<SearchSongNameCtrlD2, vk_music_fs::fs::FsUtils> SearchSongNameSingleDirD2;
    typedef vk_music_fs::fs::RemoteFileWrapper<SearchSongNameSingleDirD2, vk_music_fs::fs::FsUtils, FileManagerM> SearchSongNameRemoteFileD2;
    typedef vk_music_fs::fs::DummyDirWrapper<SearchSongNameRemoteFileD2, vk_music_fs::fs::FsUtils> SearchSongNameDummyDirD2;
    typedef vk_music_fs::AudioFs<CtrlTupleD> AudioFs;

    auto makeInj(bool createDummyDirs){
        return di::make_injector<vk_music_fs::BoundPolicy>(
                di::bind<ActTupleD>.in(di::unique),
                di::bind<RefreshActD>.in(di::extension::scoped),
                di::bind<NumberActD>.in(di::extension::scoped),

                di::bind<CtrlTupleD>.in(di::unique),
                di::bind<vk_music_fs::fs::FsUtils>.in(di::extension::scoped),
                di::bind<vk_music_fs::fs::IdGenerator>.in(di::extension::scoped),
                di::bind<vk_music_fs::fs::FsSettings>.in(di::extension::scoped),
                di::bind<FileObtainerD>.in(di::extension::scoped),
                di::bind<MyAudiosCtrlD>.in(di::extension::scoped),
                di::bind<MyAudiosSingleDirD>.in(di::extension::scoped),
                di::bind<MyAudiosDummyDirD>.in(di::extension::scoped),
                di::bind<MyAudiosRemoteFileD>.in(di::extension::scoped),
                di::bind<SearchSongNameCtrlD1>.in(di::extension::scoped),
                di::bind<SearchSongNameSingleDirD1>.in(di::extension::scoped),
                di::bind<SearchSongNameDummyDirD1>.in(di::extension::scoped),
                di::bind<SearchSongNameRemoteFileD1>.in(di::extension::scoped),
                di::bind<SearchSongNameCtrlD2>.in(di::extension::scoped),
                di::bind<SearchSongNameSingleDirD2>.in(di::extension::scoped),
                di::bind<SearchSongNameDummyDirD2>.in(di::extension::scoped),
                di::bind<SearchSongNameRemoteFileD2>.in(di::extension::scoped),
                di::bind<vk_music_fs::fs::SearchSongNameSongHelper>.in(di::extension::scoped),
                di::bind<vk_music_fs::fs::SearchSongNameArtistHelper>.in(di::extension::scoped),
                di::bind<AudioFs>.in(di::extension::scoped),
                di::bind<FileManagerM>.in(di::extension::scoped),
                di::bind<QueryMakerM>.in(di::extension::scoped),
                di::bind<vk_music_fs::fs::RootCtrl<vk_music_fs::fs::FsUtils>>.in(di::extension::scoped),
                di::bind<vk_music_fs::Mp3Extension>.to(vk_music_fs::Mp3Extension{".mp3"}),
                di::bind<vk_music_fs::NumSearchFiles>.to(vk_music_fs::NumSearchFiles{numSearchFiles}),
                di::bind<vk_music_fs::CreateDummyDirs>.to(vk_music_fs::CreateDummyDirs{createDummyDirs})
                );
    }

    di::injector<std::shared_ptr<AudioFs>, std::shared_ptr<QueryMakerM>, std::shared_ptr<FileManagerM>> inj;
    di::injector<std::shared_ptr<AudioFs>, std::shared_ptr<QueryMakerM>, std::shared_ptr<FileManagerM>> dummyInj;
    AudioFsT(): inj(makeInj(false)), dummyInj(makeInj(true)){
        setCreateDummyDirs(false);
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
                    throw vk_music_fs::net::HttpException("Test error");
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
};

TEST_F(AudioFsT, Empty){ //NOLINT
    auto api = inj.create<std::shared_ptr<AudioFs>>();
    EXPECT_EQ(api->getEntries("/").size(), 3);
    EXPECT_EQ(api->getEntries("/Search").size(), 0);
    EXPECT_EQ(api->getEntries("/Search by artist").size(), 0);
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
    api->open("/Search by artist/Artist/Artist3 - Song3.mp3");;
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