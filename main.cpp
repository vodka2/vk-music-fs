#include <HttpStream.h>
#include <FileProcessor.h>
#include <boost/di/extension/scopes/scoped.hpp>
#include <Mp3Parser.h>
#include <MusicFile.h>
#include <ThreadPool.h>
#include <FileManager.h>
#include <FileCache.h>
#include <Reader.h>
#include <Application.h>
#include <fs/AudioFs.h>
#include <SizeObtainer.h>
#include <RemoteFile.h>
#include <VkApiQueryMaker.h>
#include <ProgramOptions.h>
#include <boost/nowide/iostream.hpp>
#include <boost/nowide/args.hpp>
#include <boost/filesystem.hpp>
#include <codecvt>
#include "fuse_wrap.h"

using namespace vk_music_fs;

fuse_operations operations = {};

typedef FileProcessor<HttpStream, MusicFile, Mp3Parser, ThreadPool> FileProcessorD;
typedef AudioFs<VkApiQueryMaker> AudioFsD;
typedef FileManager<AudioFsD, FileCache, FileProcessorD, Reader> FileManagerD;
typedef Application<FileManagerD, AudioFsD> ApplicationD;

auto curTime = static_cast<uint_fast32_t>(time(nullptr)); //NOLINT

int main(int argc, char* argv[]) {
    boost::filesystem::path::imbue( std::locale( std::locale(), new std::codecvt_utf8_utf16<wchar_t>() ) );
    boost::nowide::args a(argc,argv);

    operations.init = [](fuse_conn_info *conn) {
        namespace di = boost::di;

        auto conf = reinterpret_cast<ProgramOptions*>(fuse_get_context()->private_data);

        static auto inj = di::make_injector(
            di::bind<HttpStream>.in(di::unique),
            di::bind<MusicFile>.in(di::unique),
            di::bind<Mp3Parser>.in(di::unique),
            di::bind<FileProcessorD>.in(di::unique),
            di::bind<Reader>.in(di::unique),
            di::bind<SizeObtainer>.in(di::extension::scoped),
            di::bind<HttpStreamCommon>.in(di::extension::scoped),
            di::bind<ThreadPool>.in(di::extension::scoped),
            di::bind<FileCache>.in(di::extension::scoped),
            di::bind<AudioFsD>.in(di::extension::scoped),
            di::bind<FileManagerD>.in(di::extension::scoped),
            di::bind<VkApiQueryMaker>.in(di::extension::scoped),
            di::bind<SizesCacheSize>.to(SizesCacheSize{conf->getSizesCacheSize()}),
            di::bind<FilesCacheSize>.to(FilesCacheSize{conf->getFilesCacheSize()}),
            di::bind<UserAgent>.to(UserAgent{conf->getUseragent()}),
            di::bind<Token>.to(Token{conf->getToken()}),
            di::bind<CacheDir>.to(CacheDir{conf->getCacheDir()}),
            di::bind<Mp3Extension>.to(Mp3Extension{conf->getMp3Extension()}),
            di::bind<NumSearchFiles>.to(NumSearchFiles{conf->getNumSearchFiles()}),
            di::bind<CreateDummyDirs>.to(CreateDummyDirs{conf->createDummyDirs()}),
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
        auto ptr = (void*)inj.create<ApplicationD*>();
        delete conf;
        return ptr;
    };
    operations.readdir = [](const char *path, void *buf, fuse_fill_dir_t filler, fuse_off_t offset,
                            struct fuse_file_info *fi){
        auto app = reinterpret_cast<ApplicationD*>(fuse_get_context()->private_data);
        for(const auto &entry : app->getEntries(path)){
            filler(buf, entry.c_str(), nullptr, 0);
        }
        return 0;
    };
    operations.getattr = [](const char *path, struct fuse_stat *stbuf){
        auto app = reinterpret_cast<ApplicationD*>(fuse_get_context()->private_data);
        std::fill(reinterpret_cast<uint8_t*>(stbuf), reinterpret_cast<uint8_t*>(stbuf) + sizeof(struct stat), 0);
        auto meta = app->getMeta(path);
        stbuf->st_nlink = 1;
        stbuf->st_mtim.tv_sec = curTime + meta.time;
        stbuf->st_ctim.tv_sec = curTime + meta.time;
        stbuf->st_atim.tv_sec = curTime + meta.time;
        if (meta.type == FileOrDirMeta::Type::DIR_ENTRY) {
            stbuf->st_mode = S_IFDIR | 0777u;
            return 0;
        } else if(meta.type == FileOrDirMeta::Type::FILE_ENTRY){
            stbuf->st_mode = S_IFREG | 0777u;
            stbuf->st_size = app->getFileSize(path);
            return 0;
        }
        return -ENOENT;
    };
    operations.open = [](const char *path, fuse_file_info *fi) {
        auto fname = std::string(path).substr(1);
        auto app = reinterpret_cast<ApplicationD*>(fuse_get_context()->private_data);
        auto ret = app->open(fname);
        if(ret > 0) {
            fi->fh = static_cast<unsigned>(ret);
            return 0;
        } else {
            return (int)ret;
        }
    };
    operations.rename = [](const char *oldPath, const char *newPath){
        auto app = reinterpret_cast<ApplicationD*>(fuse_get_context()->private_data);
        app->renameDir(oldPath, newPath);
        return 0;
    };
    operations.read = [](const char *path, char *buf, size_t size, fuse_off_t off, struct fuse_file_info *fi) {
        auto app = reinterpret_cast<ApplicationD*>(fuse_get_context()->private_data);
        try {
            auto bytes = app->read(static_cast<uint_fast32_t>(fi->fh), static_cast<uint_fast32_t>(off), size);
            if(bytes.empty()){
                return EOF;
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
        auto app = reinterpret_cast<ApplicationD*>(fuse_get_context()->private_data);
        app->createDir(path);
        return 0;
    };
    operations.rmdir = [](const char *path){
        auto app = reinterpret_cast<ApplicationD*>(fuse_get_context()->private_data);
        app->deleteDir(path);
        return 0;
    };
    operations.unlink = [](const char *path){
        auto app = reinterpret_cast<ApplicationD*>(fuse_get_context()->private_data);
        app->deleteFile(path);
        return 0;
    };

    auto opts = new ProgramOptions(static_cast<uint_fast32_t>(argc), argv, "VkMusicFs.ini", "VkMusicFs");
    if(opts->needHelp()){
        boost::nowide::cerr << "Usage " << argv[0] << " mountpoint [options]\n" << std::endl;
        boost::nowide::cerr << opts->getHelpString() << std::endl;
    }

    return fuse_main(static_cast<int>(opts->getFuseArgc()), opts->getFuseArgv(), &operations, opts);
}