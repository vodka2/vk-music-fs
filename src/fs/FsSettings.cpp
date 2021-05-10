#include "FsSettings.h"

using namespace vk_music_fs;
using namespace vk_music_fs::fs;

uint_fast32_t FsSettings::getNumSearchFiles() const {
    return _numSearchFiles;
}

const std::string FsSettings::getMp3Ext() const {
    return _mp3Ext;
}

bool FsSettings::isCreateDummyDirs() {
    return _createDummyDirs;
}

FsSettings::FsSettings(
        const NumSearchFiles &numSearchFiles,
        const Mp3Extension &mp3Ext,
        const CreateDummyDirs &createDummyDirs,
        const PathToFs &pathToFs,
        const UseAsyncNotifier &useAsyncNotifier,
        const PhotoName &photoName
) :
_numSearchFiles(numSearchFiles), _mp3Ext(mp3Ext), _pathToFs(pathToFs),
_createDummyDirs(createDummyDirs), _useAsyncNotifier(useAsyncNotifier), _photoName(photoName){
}

const std::string FsSettings::getPathToFs() const {
    return _pathToFs;
}

bool FsSettings::isUseAsyncNotifier() {
    return _useAsyncNotifier;
}

const std::string &FsSettings::getPhotoName() const {
    return _photoName;
}
