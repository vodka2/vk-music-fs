#include <common/MusicFsException.h>

vk_music_fs::MusicFsException::MusicFsException(const std::string &arg) : runtime_error(arg) {}
