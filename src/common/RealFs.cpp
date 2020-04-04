#include "RealFs.h"
#include <boost/nowide/fstream.hpp>

using namespace vk_music_fs;

void RealFs::createFile(const std::string &filename) {
    boost::nowide::ofstream file{filename};
}
