#include "FsSettings.h"

using namespace vk_music_fs;
using namespace vk_music_fs::fs;

uint_fast32_t FsSettings::getNumSearchFiles() const {
    return _numSearchFiles;
}

const std::string FsSettings::getMp3Ext() const {
    return _mp3Ext;
}

bool FsSettings::isCreateDummyDirs() const {
    return _createDummyDirs;
}

FsSettings::FsSettings(
        const NumSearchFiles &numSearchFiles,
        const Mp3Extension &mp3Ext,
        const CreateDummyDirs &createDummyDirs
) : _numSearchFiles(numSearchFiles), _mp3Ext(mp3Ext), _createDummyDirs(createDummyDirs){
}
