#pragma once

#include <common/common.h>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>

namespace vk_music_fs {
    class ProgramOptions {
    public:
        ProgramOptions(uint_fast32_t argc, char **argv, const std::string &configName, const std::string &appName);

        virtual ~ProgramOptions();

        VkCredentials getCredentials();
        bool needObtainToken();

        std::string getUseragent();
        bool needHelp();
        bool needClearCache();
        std::string getHelpString();
        std::string getToken();
        std::string getMp3Extension();
        std::string getCacheDir();
        std::string getErrLogFile();
        std::string getMountPoint();
        bool createDummyDirs();
        bool logErrorsToFile();
        uint_fast32_t getFilesCacheSize();
        uint_fast32_t getSizesCacheSize();
        uint_fast32_t getNumSearchFiles();
        uint_fast32_t getNumSizeRetries();
        uint_fast32_t getHttpTimeout();

        char **getFuseArgv();
        uint_fast32_t getFuseArgc();
    private:
        void createFuseArgv(char **argv, const std::vector<std::string> &additionalParameters);
        void addCommonOpts(boost::program_options::options_description &opts, const std::string &appName);
        void addTokenOpts(boost::program_options::options_description &opts);
        void addCmdlineOnlyOpts(boost::program_options::options_description &opts);
        std::string getUserConfigDir(const std::string &appName);
        std::string getUserCacheDir(const std::string &appName);
        bool createDummyDirsDefault();
        void parseOptions(boost::program_options::variables_map &vm);
        void parseCommonOptions(boost::program_options::variables_map &vm);
        void parseMountPoint(const std::vector<std::string> &opts);
        std::optional<VkCredentials> _creds;
        std::string _helpStr;
        std::string _errLogFile;
        bool _logErrorsToFile;
        bool _createDummyDirs;
        bool _needHelp;
        bool _needClearCache;
        bool _argvCreated;
        char **_fuseArgv;
        uint_fast32_t _fuseArgc;
        std::string _useragent;
        std::string _token;
        std::string _mp3Ext;
        std::string _cacheDir;
        std::string _mountPoint;
        uint_fast32_t _filesCacheSize;
        uint_fast32_t _sizesCacheSize;
        uint_fast32_t _numSearchFiles;
        uint_fast32_t _numSizeRetries;
        uint_fast32_t _httpTimeout;
    };
}
