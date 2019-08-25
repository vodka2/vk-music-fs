#pragma once

#include "common.h"
#include <common/MusicFsException.h>
#include <boost/nowide/fstream.hpp>
#include <mutex>

namespace vk_music_fs {
    class ErrLogger {
    public:
        ErrLogger(const ErrLogFile &errLogFile, const LogErrorsToFile &logErrorsToFile);
        void logException(const MusicFsException &exc);

    private:
        std::string getTimeAndDate();
        std::mutex _writeMutex;
        std::string _errLogFile;
        bool _logErrorsToFile;
    };
}
