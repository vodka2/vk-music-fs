#include "DirOrFile.h"
#include "File.h"
#include "Dir.h"

using namespace vk_music_fs;
using namespace fs;

DirOrFile::DirOrFile(const DirPtr &dir) : _data(dir){
}

DirOrFile::DirOrFile(const FilePtr &file) : _data(file){
}

bool DirOrFile::isDir() const{
    return std::holds_alternative<DirPtr>(_data);
}

bool DirOrFile::isFile() const{
    return std::holds_alternative<FilePtr>(_data);
}

DirPtr DirOrFile::dir() const{
    return std::get<DirPtr>(_data);
}

FilePtr DirOrFile::file() const{
    return std::get<FilePtr>(_data);
}

std::string DirOrFile::getName() const {
    if(isDir()){
        return dir()->getName();
    } else {
        return file()->getName();
    }
}
