#include "ErrLogger.h"
#include <boost/nowide/iostream.hpp>
#include <iomanip>

using namespace vk_music_fs;

ErrLogger::ErrLogger(const ErrLogFile &errLogFile, const LogErrorsToFile &logErrorsToFile):
_errLogFile(errLogFile.t), _logErrorsToFile(logErrorsToFile.t){
}

void ErrLogger::logException(const MusicFsException &exc) {
    std::scoped_lock<std::mutex> lock(_writeMutex);
    auto curTD = getTimeAndDate();
    if(_logErrorsToFile){
        boost::nowide::ofstream strm{_errLogFile.c_str(), std::ios::app};
        strm << curTD << exc.what() << std::endl;
    }
    boost::nowide::cerr << curTD << exc.what() << std::endl;
}

std::string ErrLogger::getTimeAndDate() {
    auto now = std::chrono::system_clock::now();
    auto inTime = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::put_time(std::localtime(&inTime), "%c ");
    return ss.str();
}
