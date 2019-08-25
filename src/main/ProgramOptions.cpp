#include "ProgramOptions.h"
#include <common/MusicFsException.h>
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

ProgramOptions::ProgramOptions(uint_fast32_t argc, char **argv, const std::string &configName, const std::string &appName):
_argvCreated(false), _creds(std::nullopt){
    po::options_description generalDesc("General options");
    addCommonOpts(generalDesc, appName);

    po::options_description tokenDesc("Token options");
    addTokenOpts(tokenDesc);

    po::options_description allDesc("");
    addCmdlineOnlyOpts(allDesc);
    allDesc.add(tokenDesc).add(generalDesc);

    std::vector<std::string> additionalParameters;
    po::variables_map vm;
    try {
        auto opts = po::command_line_parser(static_cast<int>(argc), argv)
                .options(allDesc).
                allow_unregistered().run();
        additionalParameters = po::collect_unrecognized(opts.options, po::include_positional);
        po::store(opts, vm);
        po::notify(vm);
    } catch (const po::error &err){
        throw MusicFsException(std::string("Error when parsing command line options: ") + err.what());
    }

    addCmdlineOnlyOpts(generalDesc);

    _needHelp = vm.count("help") != 0;
    _needClearCache = vm.count("clear_cache") != 0;
    if(_needHelp){
        std::stringstream strm;
        strm << generalDesc << tokenDesc;
        _helpStr = strm.str();
        additionalParameters.emplace_back("-ho");
    } else if(vm.count("get_token")){
        if (!vm["get_token"].empty()) {
            auto loginPass = vm["get_token"].as<std::vector<std::string>>();
            if(loginPass.size() == 2){
                _creds = {loginPass[0], loginPass[1]};
                parseCommonOptions(vm);
                return;
            } else {
                throw MusicFsException("You must specify login and password after '--get_token'");
            }
        }
    } else if(_needClearCache) {
        parseCommonOptions(vm);
        return;
    }

    createFuseArgv(argv, additionalParameters);

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
            try {
                boost::nowide::ifstream strm(p.string().c_str());
                po::store(po::parse_config_file<char>(strm, generalDesc), vm);
                notify(vm);
            } catch (const po::error &err){
                throw MusicFsException("Error when parsing config " + p.string() + ". " + err.what());
            }
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

    parseCommonOptions(vm);
}

void ProgramOptions::parseCommonOptions(boost::program_options::variables_map &vm) {
    _sizesCacheSize = vm["sizes_cache_size"].as<uint_fast32_t>();
    _filesCacheSize = vm["files_cache_size"].as<uint_fast32_t>();
    _mp3Ext = vm["mp3_ext"].as<std::string>();
    _numSearchFiles = vm["num_search_files"].as<uint_fast32_t>();
    _cacheDir = vm["cache_dir"].as<std::string>();
    _errLogFile = vm["err_log"].as<std::string>();
    _createDummyDirs = vm["create_dummy_dirs"].as<bool>();
    _logErrorsToFile = vm["log_err_to_file"].as<bool>();
    _numSizeRetries = vm["num_size_retries"].as<uint_fast32_t>();
    _httpTimeout = vm["http_timeout"].as<uint_fast32_t>();
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
#if defined(WIN32) || defined(__APPLE__)
            true
#else
            false
#endif
    ;
}

uint_fast32_t ProgramOptions::getNumSizeRetries() {
    return _numSizeRetries;
}

ProgramOptions::~ProgramOptions() {
    if(_argvCreated) {
        for (uint_fast32_t i = 1; i < _fuseArgc; i++) {
            free(_fuseArgv[i]);
        }
        delete[] _fuseArgv;
    }
}

std::string ProgramOptions::getErrLogFile() {
    return _errLogFile;
}

bool ProgramOptions::logErrorsToFile() {
    return _logErrorsToFile;
}

uint_fast32_t ProgramOptions::getHttpTimeout() {
    return _httpTimeout;
}

void ProgramOptions::addCommonOpts(boost::program_options::options_description &opts, const std::string &appName) {
    opts.add_options()
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
            ("num_size_retries", po::value<uint_fast32_t>()->default_value(3),
             "set max number of HEAD requests when retrieving size")
            ("err_log", po::value<std::string>()->default_value(
                    (bfs::path(getUserConfigDir(appName)) / "ErrorLog.txt").string()
            ), "set error log file name")
            ("log_err_to_file", po::value<bool>()->default_value(false), "log errors to file")
            ("http_timeout", po::value<uint_fast32_t>()->default_value(12000),
             "set HTTP requests timeout in milliseconds");
}

void ProgramOptions::addTokenOpts(boost::program_options::options_description &opts) {
    opts.add_options()
            ("get_token",
             po::value<std::vector<std::string>>()->multitoken(),
             "Obtain token by login and password")
            ;
}

void ProgramOptions::addCmdlineOnlyOpts(boost::program_options::options_description &opts) {
    opts.add_options()
            ("help", "produce help message")
            ("clear_cache", "clear remote files and remote file sizes cache");
}

void ProgramOptions::createFuseArgv(char **argv, const std::vector<std::string> &additionalParameters) {
    _argvCreated = true;
    _fuseArgv = new char*[additionalParameters.size() + 2];
    _fuseArgv[0] = argv[0];

    uint_fast32_t i = 1;
    for (const auto &param : additionalParameters) {
        _fuseArgv[i] = strdup(param.c_str());
        i++;
    }
    _fuseArgc = i;
    _fuseArgv[i] = nullptr;
}

VkCredentials ProgramOptions::getCredentials() {
    return *_creds;
}

bool ProgramOptions::needObtainToken() {
    return static_cast<bool>(_creds);
}

bool ProgramOptions::needClearCache() {
    return _needClearCache;
}
