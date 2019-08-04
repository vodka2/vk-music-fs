#include <net/HttpStream.h>
#include <FileProcessor.h>
#include <boost/di/extension/scopes/scoped.hpp>
#include <Mp3Parser.h>
#include <MusicFile.h>
#include <FileManager.h>
#include <Reader.h>
#include <Application.h>
#include <fs/AudioFs.h>
#include <fs/FsUtils.h>
#include <net/VkApiQueryMaker.h>
#include <boost/nowide/iostream.hpp>
#include <boost/nowide/args.hpp>
#include <ErrLogger.h>
#include <codecvt>
#include <boost/filesystem/path.hpp>
#include <token/TokenReceiver.h>
#include <fs/ctrl/DummyDirWrapper.h>
#include <fs/ctrl/RemoteFileWrapper.h>
#include <fs/ctrl/MyAudiosCtrl.h>
#include <fs/ctrl/SearchSongNameCtrl.h>
#include <fs/ctrl/RootCtrl.h>
#include <fs/ctrl/SingleDirCtrl.h>
#include <fs/FileObtainer.h>
#include <fs/ctrl/SearchSongNameArtistHelper.h>
#include <fs/ctrl/SearchSongNameSongHelper.h>
#include <diext/common_di.h>
#include "fuse_wrap.h"

using namespace vk_music_fs;

fuse_operations operations = {};

typedef FileProcessor<net::HttpStream, MusicFile, Mp3Parser, ThreadPool> FileProcessorD;
typedef FileManager<FileCache, FileProcessorD, Reader> FileManagerD;
typedef Application<
        FileManagerD,
        AudioFs<fs::CtrlTuple<fs::FsUtils, fs::FileObtainer<net::VkApiQueryMaker>, FileManagerD>>,
        ErrLogger, net::HttpStreamCommon
> ApplicationD;

auto curTime = static_cast<uint_fast32_t>(time(nullptr)); //NOLINT

auto commonInj = [] (const std::shared_ptr<ProgramOptions> &conf){ // NOLINT
    namespace di = boost::di;
    return makeStorageInj(
            di::bind<net::HttpStream>.in(di::unique),
            di::bind<MusicFile>.in(di::unique),
            di::bind<Mp3Parser>.in(di::unique),
            di::bind<FileProcessorD>.in(di::unique),
            di::bind<Reader>.in(di::unique),
            di::bind<TagSizeCalculator>.in(di::unique),

            di::bind<SizesCacheSize>.to(SizesCacheSize{conf->getSizesCacheSize()}),
            di::bind<FilesCacheSize>.to(FilesCacheSize{conf->getFilesCacheSize()}),
            di::bind<CacheDir>.to(CacheDir{conf->getCacheDir()}),
            di::bind<Mp3Extension>.to(Mp3Extension{conf->getMp3Extension()}),
            di::bind<NumSearchFiles>.to(NumSearchFiles{conf->getNumSearchFiles()}),
            di::bind<CreateDummyDirs>.to(CreateDummyDirs{conf->createDummyDirs()}),
            di::bind<NumSizeRetries>.to(NumSizeRetries{conf->getNumSizeRetries()}),
            di::bind<ProgramOptions>.to(conf),
            di::bind<LogErrorsToFile>.to(LogErrorsToFile{conf->logErrorsToFile()}),
            di::bind<ErrLogFile>.to(ErrLogFile{conf->getErrLogFile()}),
            di::bind<HttpTimeout>.to(HttpTimeout{conf->getHttpTimeout()}),
            di::bind<di::extension::iextfactory<FileProcessorD,
                    Artist,
                    Title,
                    Mp3Uri,
                    TagSize,
                    RemoteFile,
                    CachedFilename
            >>.to(di::extension::extfactory<FileProcessorD>{}),
            di::bind<di::extension::iextfactory<Reader,
                    CachedFilename,
                    FileSize
            >>.to(di::extension::extfactory<Reader>{})
    );
};

auto tokenUserAgentInj = [] (const std::shared_ptr<ProgramOptions> &conf){ //NOLINT
    namespace di = boost::di;
    return di::make_injector<vk_music_fs::CustomScopePolicy>(
            di::bind<UserAgent>.to(UserAgent{conf->getUseragent()}),
            di::bind<Token>.to(Token{conf->getToken()})
    );
};

auto getTokenInj = [] (const VkCredentials &creds, const std::string &userAgent){ //NOLINT
    namespace di = boost::di;
    return di::make_injector<vk_music_fs::CustomScopePolicy>(
            di::bind<UserAgent>.to(UserAgent{userAgent}),
            di::bind<VkCredentials>.to(creds)
    );
};

int printToken(const VkCredentials &creds, const std::shared_ptr<ProgramOptions> &opts){
    namespace di = boost::di;
    auto inj = di::make_injector<vk_music_fs::CustomScopePolicy>(
            commonInj(opts),
            getTokenInj(
                    creds,
                    token::TokenReceiver::getUserAgent()
            )
    );
    auto obtainer = inj.create<std::shared_ptr<token::TokenReceiver>>();
    int returnStatus;
    try {
        auto token = obtainer->getToken();
        boost::nowide::cout << "token=" << token << std::endl;
        boost::nowide::cout << "user_agent=" <<  token::TokenReceiver::getUserAgent() << std::endl;
        returnStatus = 0;
    } catch (const MusicFsException &ex){
        boost::nowide::cerr << ex.what() << std::endl;
        returnStatus = 1;
    }
    inj.create<std::shared_ptr<net::HttpStreamCommon>>()->stop();
    return returnStatus;
}

int clearCache(const std::shared_ptr<ProgramOptions> &opts){
    namespace di = boost::di;
    commonInj(opts).create<std::shared_ptr<CacheSaver>>()->clearCache();
    return 0;
}

int main(int argc, char* argv[]) {
    boost::filesystem::path::imbue( std::locale( std::locale(), new std::codecvt_utf8_utf16<wchar_t>() ) );
    boost::nowide::args a(argc,argv);

    operations.init = [](fuse_conn_info *conn) {
        namespace di = boost::di;

        auto conf = *reinterpret_cast<std::shared_ptr<ProgramOptions>*>(fuse_get_context()->private_data);
        static auto inj = di::make_injector<vk_music_fs::CustomScopePolicy>(
            commonInj(conf),
            tokenUserAgentInj(conf)
        );
        auto ptr = (void*)inj.create<ApplicationD*>();
        return ptr;
    };
    operations.destroy = [] (void *data){
        auto app = reinterpret_cast<ApplicationD*>(data);
        delete app;
    };
    operations.readdir = [](const char *path, void *buf, fuse_fill_dir_t filler, fuse_off_t offset,
                            struct fuse_file_info *fi){
        try {
            auto app = reinterpret_cast<ApplicationD *>(fuse_get_context()->private_data);
            for(const auto &entry : app->getEntries(path)){
                filler(buf, entry.c_str(), nullptr, 0);
            }
            return 0;
        } catch (...){
            return -ENOENT;
        }
    };
    operations.getattr = [](const char *path, struct fuse_stat *stbuf){
        auto app = reinterpret_cast<ApplicationD*>(fuse_get_context()->private_data);
        std::fill(reinterpret_cast<uint8_t*>(stbuf), reinterpret_cast<uint8_t*>(stbuf) + sizeof(struct fuse_stat), 0);
        try {
            auto meta = app->getMeta(path);
            stbuf->st_nlink = 1;
            stbuf->st_mtim.tv_sec = curTime + meta.time;
            stbuf->st_ctim.tv_sec = curTime + meta.time;
            stbuf->st_atim.tv_sec = curTime + meta.time;
            if (meta.type == FileOrDirMeta::Type::DIR_ENTRY) {
                stbuf->st_mode = static_cast<uint_fast32_t>(S_IFDIR) | 0777u;
                return 0;
            } else if (meta.type == FileOrDirMeta::Type::FILE_ENTRY) {
                stbuf->st_mode = static_cast<uint_fast32_t>(S_IFREG) | 0777u;
                stbuf->st_size = app->getFileSize(path);
                return 0;
            }
        } catch (...){
            return -EACCES;
        }
        return -ENOENT;
    };
    operations.open = [](const char *path, fuse_file_info *fi) {
        try {
            auto app = reinterpret_cast<ApplicationD *>(fuse_get_context()->private_data);
            fi->fh = static_cast<unsigned>(app->open(path));
            return 0;
        } catch (...){
            return -EACCES;
        }
    };
    operations.rename = [](const char *oldPath, const char *newPath){
        try {
            auto app = reinterpret_cast<ApplicationD *>(fuse_get_context()->private_data);
            app->rename(oldPath, newPath);
        } catch (...){
            return -EACCES;
        }
        return 0;
    };
    operations.read = [](const char *path, char *buf, size_t size, fuse_off_t off, struct fuse_file_info *fi) {
        auto app = reinterpret_cast<ApplicationD*>(fuse_get_context()->private_data);
        try {
            auto bytes = app->read(static_cast<uint_fast32_t>(fi->fh), static_cast<uint_fast32_t>(off), size);
            if(bytes.empty()){
                return 0;
            }
            std::copy(bytes.cbegin(), bytes.cend(), buf);
            return (int)bytes.size();
        } catch (...){
            return -EIO;
        }
    };
    operations.release = [](const char *path, struct fuse_file_info *fi){
        auto app = reinterpret_cast<ApplicationD*>(fuse_get_context()->private_data);
        app->close(static_cast<uint_fast32_t>(fi->fh));
        return 0;
    };
    operations.mkdir = [](const char *path, fuse_mode_t){
        try {
            auto app = reinterpret_cast<ApplicationD *>(fuse_get_context()->private_data);
            app->createDir(path);
            return 0;
        } catch (...){
            return -EACCES;
        }
    };
    operations.rmdir = [](const char *path){
        try {
            auto app = reinterpret_cast<ApplicationD *>(fuse_get_context()->private_data);
            app->deleteDir(path);
            return 0;
        } catch (...){
            return -ENOENT;
        }
    };
    operations.unlink = [](const char *path){
        try {
            auto app = reinterpret_cast<ApplicationD *>(fuse_get_context()->private_data);
            app->deleteFile(path);
            return 0;
        } catch (...){
            return -ENOENT;
        }
    };
    std::shared_ptr<ProgramOptions> opts;
    try {
        opts = std::make_shared<ProgramOptions>(static_cast<uint_fast32_t>(argc), argv, "VkMusicFs.ini", "VkMusicFs");
    } catch (const MusicFsException& exc){
        boost::nowide::cerr << exc.what() << std::endl;
        return 1;
    }
    if(opts->needHelp()){
        boost::nowide::cerr << "Usage " << argv[0] << " mountpoint [options]\n" << std::endl;
        boost::nowide::cerr << opts->getHelpString() << std::endl;
    } else {
        if (opts->needObtainToken()) {
            return printToken(opts->getCredentials(), opts);
        }
        if (opts->needClearCache()) {
            return clearCache(opts);
        }
    }
    return fuse_main(static_cast<int>(opts->getFuseArgc()), opts->getFuseArgv(), &operations, &opts);
}