#pragma once
#include <common/common.h>
#include <memory>
#include <optional>
#include <boost/algorithm/string.hpp>

namespace vk_music_fs {
    template <typename TAudioFs>
    class FakeFs {
    public:
        FakeFs(
                std::shared_ptr<TAudioFs> realFs, Mp3Extension mp3Ext,
                CreateDummyDirs createDummyDirs, fs::UseAsyncNotifier useAsyncNotifier
        ) : _realFs(realFs), _mp3Ext(mp3Ext.t), _createDummyDirs(createDummyDirs), _useAsyncNotifier(useAsyncNotifier) {}

        int_fast32_t open(const std::string &filename){
            auto fakeDirData = isFakeDir(filename);
            if (fakeDirData) {
                if (fakeDirData->type == FakeDirData::FakeType::MP3) {
                    return _realFs->open(fakeDirData->realPath);
                } else if (fakeDirData->type == FakeDirData::FakeType::META) {
                    return 1;
                } else {
                    return _realFs->open(fakeDirData->realPath);
                }
            } else {
                return _realFs->open(filename);
            }
        }

        FileOrDirMeta getMeta(const std::string &path){
            auto fakeDirData = isFakeDir(path);
            if (fakeDirData) {
                if (fakeDirData->type == FakeDirData::FakeType::MP3) {
                    return _realFs->getMeta(fakeDirData->realPath);
                } else if (fakeDirData->type == FakeDirData::FakeType::META) {
                    auto meta = _realFs->getMeta(fakeDirData->realPath);
                    if (meta.type == FileOrDirMeta::Type::FILE_ENTRY) {
                        meta.type = FileOrDirMeta::Type::FILE_ENTRY_NO_SIZE;
                    }
                    return meta;
                } else {
                    auto meta = _realFs->getMeta(fakeDirData->realPath);
                    if (meta.type == FileOrDirMeta::Type::FILE_ENTRY) {
                        meta.type = FileOrDirMeta::Type::DIR_ENTRY;
                    }
                    return meta;
                }
            } else {
                return _realFs->getMeta(path);
            }
        }

        uint_fast32_t getFileSize(const std::string &path){
            auto fakeDirData = isFakeDir(path);
            if (fakeDirData && fakeDirData->type == FakeDirData::FakeType::MP3) {
                return _realFs->getFileSize(fakeDirData->realPath);
            } else {
                return _realFs->getFileSize(path);
            }
        }

        std::vector<std::string> getEntries(const std::string &dirPath){
            auto fakeDirData = isFakeDir(dirPath);
            if (fakeDirData && fakeDirData->type == FakeDirData::FakeType::OTHER) {
                return std::move(_realFs->getEntries(fakeDirData->realPath));
            } else {
                return std::move(_realFs->getEntries(dirPath));
            }
        }

        void createDir(const std::string &dirPath) {
            auto fakeDirData = isFakeDir(dirPath);
            if (fakeDirData && fakeDirData->type == FakeDirData::FakeType::OTHER) {
                TempOverride dummyDirsLock{_createDummyDirs, false};
                TempOverride useAsyncNotifierLock{_useAsyncNotifier, false};
                return _realFs->createDir(fakeDirData->realPath);
            } else {
                return _realFs->createDir(dirPath);
            }
        }

        void rename(const std::string &oldPath, const std::string &newPath){
            auto fakeDirOldData = isFakeDir(oldPath);
            auto fakeDirNewData = isFakeDir(newPath);
            if (
                    fakeDirOldData && fakeDirOldData->type == FakeDirData::FakeType::OTHER &&
                    fakeDirNewData && fakeDirNewData->type == FakeDirData::FakeType::OTHER
            ) {
                TempOverride dummyDirsLock{_createDummyDirs, false};
                TempOverride useAsyncNotifierLock{_useAsyncNotifier, false};
                return _realFs->rename(fakeDirOldData->realPath, fakeDirNewData->realPath);
            } else {
                return _realFs->rename(oldPath, newPath);
            }
        }
    private:
        struct FakeDirData {
            enum class FakeType {
                META,
                MP3,
                OTHER
            } type;
            std::string realPath;
        };
        std::optional<FakeDirData> isFakeDir(const std::string &filename) const {
            std::string fakePath = "/.fake";
            if (boost::starts_with(filename, fakePath)) {
                auto filenameNoFake = filename.substr(fakePath.size());
                std::string fileMp3 = "/file" + _mp3Ext;
                std::string fileMeta = "/meta" + _mp3Ext;
                FakeDirData ret;
                if (boost::ends_with(filenameNoFake, fileMp3)) {
                    ret.type = FakeDirData::FakeType::MP3;
                    ret.realPath = filenameNoFake.substr(0, filenameNoFake.size() - fileMp3.size());
                } else if (boost::ends_with(filenameNoFake, fileMeta)) {
                    ret.type = FakeDirData::FakeType::META;
                    ret.realPath = filenameNoFake.substr(0, filenameNoFake.size() - fileMeta.size());
                } else {
                    ret.type = FakeDirData::FakeType::OTHER;
                    ret.realPath = filenameNoFake;
                }
                return ret;
            } else {
                return std::nullopt;
            }
        }
        std::shared_ptr<TAudioFs> _realFs;
        std::string _mp3Ext;
        CreateDummyDirs _createDummyDirs;
        fs::UseAsyncNotifier _useAsyncNotifier;
    };
}
