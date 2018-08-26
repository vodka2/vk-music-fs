#include "HttpException.h"

vk_music_fs::net::HttpException::HttpException(std::string str):std::runtime_error(str){
}
