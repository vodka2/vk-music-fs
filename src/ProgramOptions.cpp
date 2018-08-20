#include "ProgramOptions.h"
#include <fstream>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/filesystem.hpp>
#include <cfgpath.h>
#include <codecvt>
#include <boost/nowide/iostream.hpp>
#include <boost/nowide/fstream.hpp>

using namespace vk_music_fs;

namespace po = boost::program_options;
namespace bfs = boost::filesystem;

ProgramOptions::ProgramOptions(uint_fast32_t argc, char **argv, const std::string &configName, const std::string &appName) {
    po::options_description desc("VK Music FS options");

    desc.add_options()
            ("help", "produce help message")
            ("token", po::value<std::string>(), "set token")
            ("user_agent", po::value<std::string>(), "set user agent")
            ("sizes_cache_size", po::value<uint_fast32_t>()->default_value(10000),
                    "set max number of remote file sizes in cache")
            ("files_cache_size", po::value<uint_fast32_t>()->default_value(300),
                    "set max number of remote files in cache")
            ("mp3_ext", po::value<std::string>()->default_value(".mp3"), "set mp3 files extension")
            ("num_search_files", po::value<uint_fast32_t>()->default_value(10),
                    "set initial number of files in the search directory")
            ("cache_dir", po::value<std::string>()->default_value(getUserCacheDir(appName)), "set cache dir")
            ("create_dummy_dirs", po::value<bool>()->default_value(createDummyDirsDefault()), "create dummy dirs")
    ;

    po::variables_map vm;
    auto opts = po::command_line_parser(static_cast<int>(argc), argv).options(desc).allow_unregistered().run();
    auto additionalParameters = po::collect_unrecognized(opts.options, po::include_positional);
    po::store(opts, vm);
    po::notify(vm);

    _needHelp = vm.count("help") != 0;
    if(_needHelp){
        std::stringstream strm;
        strm << desc;
        _helpStr = strm.str();
        additionalParameters.emplace_back("-ho");
    }

    _fuseArgv = new char*[additionalParameters.size() + 2];
    _fuseArgv[0] = argv[0];

    uint_fast32_t i = 1;
    for (const auto &param : additionalParameters) {
        _fuseArgv[i] = strdup(param.c_str());
        i++;
    }
    _fuseArgc = i;
    _fuseArgv[i] = nullptr;

    if(_needHelp){
        return;
    }

    auto curPath = bfs::system_complete(bfs::path(argv[0])).parent_path();

    std::vector<bfs::path> paths = {bfs::path(getUserConfigDir(appName)) / configName, curPath / configName};

#ifdef __linux__
    paths.push_back(bfs::path("/etc") / configName);
#endif

    for(const auto &p: paths){
        if(bfs::exists(p)){
            boost::nowide::ifstream strm(p.string().c_str());
            po::store(po::parse_config_file<char>(strm, desc), vm);
            notify(vm);
        }
    }
    parseOptions(vm);
}

std::string ProgramOptions::getUseragent() {
    return _useragent;
}

std::string ProgramOptions::getToken() {
    return _token;
}

char **ProgramOptions::getFuseArgv() {
    return _fuseArgv;
}

uint_fast32_t ProgramOptions::getFuseArgc() {
    return _fuseArgc;
}

bool ProgramOptions::needHelp() {
    return _needHelp;
}

std::string ProgramOptions::getHelpString() {
    return _helpStr;
}

std::string ProgramOptions::getMp3Extension() {
    return _mp3Ext;
}

uint_fast32_t ProgramOptions::getFilesCacheSize() {
    return _filesCacheSize;
}

uint_fast32_t ProgramOptions::getSizesCacheSize() {
    return _sizesCacheSize;
}

uint_fast32_t ProgramOptions::getNumSearchFiles() {
    return _numSearchFiles;
}

void ProgramOptions::parseOptions(boost::program_options::variables_map &vm) {
    if(vm.count("token")){
        _token = vm["token"].as<std::string>();
    }
    if(vm.count("user_agent")){
        _useragent = vm["user_agent"].as<std::string>();
    }

    _sizesCacheSize = vm["sizes_cache_size"].as<uint_fast32_t>();
    _filesCacheSize = vm["files_cache_size"].as<uint_fast32_t>();
    _mp3Ext = vm["mp3_ext"].as<std::string>();
    _numSearchFiles = vm["num_search_files"].as<uint_fast32_t>();
    _cacheDir = vm["cache_dir"].as<std::string>();
    _createDummyDirs = vm["create_dummy_dirs"].as<bool>();
}

std::string ProgramOptions::getCacheDir() {
    return _cacheDir;
}

bool ProgramOptions::createDummyDirs() {
    return _createDummyDirs;
}

std::string ProgramOptions::getUserConfigDir(const std::string &appName) {
    char path[MAX_PATH];
    get_user_config_folder(path, MAX_PATH, appName.c_str());
    return std::string(path);
}

std::string ProgramOptions::getUserCacheDir(const std::string &appName) {
    char path[MAX_PATH];
    get_user_cache_folder(path, MAX_PATH, appName.c_str());
    return std::string(path);
}

bool ProgramOptions::createDummyDirsDefault() {
    return
#ifdef WIN32
            true
#else
            false
#endif
    ;
}
