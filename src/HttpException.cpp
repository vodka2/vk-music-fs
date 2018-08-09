#include "HttpException.h"

vk_music_fs::HttpException::HttpException(std::string str):std::runtime_error(str){
}
