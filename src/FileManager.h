#pragma once

#include <common.h>
#include <memory>
#include <boost/di.hpp>
#include <unordered_set>
#include <unordered_map>
#include <variant>
#include <boost/di/extension/injections/extensible_injector.hpp>
#include "RemoteFile.h"
#include "IFileManager.h"
#include <mutex>

namespace vk_music_fs {
    template <typename TVkApi, typename TFileCache, typename TFileProcessor, typename TReader, typename TFact>
    class FileManager: public IFileManager {
    public:
        FileManager(
                const std::shared_ptr<TVkApi> &api,
                const std::shared_ptr<TFileCache> &fileCache,
                const TFact &injector
        ) :_api(api), _fileCache(fileCache),
        _injector(injector){
        }
        int_fast32_t open(const std::string &filename) override{
            namespace di = boost::di;
            RemoteFile remFile = _api->getRemoteFile(filename);
            std::scoped_lock<std::mutex> procsLock(_procsMutex);
            std::scoped_lock<std::mutex> readersLock(_readersMutex);
            uint_fast32_t retId = _idToRemFile.size() + 1;
            _idToRemFile.insert(std::make_pair<>(retId, remFile));
            if(_procs.find(remFile) != _procs.end()){
                _procs[remFile].ids.insert(retId);
                _readers[remFile].ids.insert(std::make_pair<>(retId, _procs[remFile].proc));
            } else {
                auto fname = _fileCache->getFilename(remFile);
                if(fname.inCache){
                    auto reader = di::make_injector(
                            di::extension::make_extensible(*_injector),
                            di::bind<CachedFilename>.to(CachedFilename{fname.data}),
                            di::bind<FileSize>.to(FileSize{_fileCache->getFileSize(remFile)})
                    ).template create<std::shared_ptr<TReader>>();
                    _readers[remFile].ids.insert(std::make_pair<>(retId, reader));
                } else {
                    _procs[remFile].proc = di::make_injector(
                            di::extension::make_extensible(*_injector),
                            di::bind<Artist>.to(Artist{remFile.getArtist()}),
                            di::bind<Title>.to(Title{remFile.getTitle()}),
                            di::bind<Mp3Uri>.to(Mp3Uri{remFile.getUri()}),
                            di::bind<TagSize>.to(TagSize(_fileCache->getTagSize(remFile))),
                            di::bind<CachedFilename>.to(CachedFilename{fname.data})
                    ).template create<std::shared_ptr<TFileProcessor>>();
                    _readers[remFile].ids.insert(std::make_pair<>(retId, _procs[remFile].proc));
                    _procs[remFile].ids.insert(retId);
                }
                _readers[remFile].fname = fname.data;
            }
            return retId;
        }
        ByteVect read(uint_fast32_t id, uint_fast32_t offset, uint_fast32_t size) override{
            ByteVect ret;
            std::variant<std::shared_ptr<TReader>, std::shared_ptr<TFileProcessor>> reader;
            std::scoped_lock<std::mutex> _lock(_readersMutex);
            reader = _readers[_idToRemFile.find(id)->second].ids.find(id)->second;
            std::visit([id, offset, size, &ret](auto &&el) {
                ret = std::move(el->read(offset, size));
            }, reader);
            return std::move(ret);
        }
        void close(uint_fast32_t id) override{
            std::scoped_lock<std::mutex> procsLock(_procsMutex);
            std::scoped_lock<std::mutex> readersLock(_readersMutex);
            auto remFile = _idToRemFile.find(id)->second;
            if(_procs.find(remFile) != _procs.end()) {
                _procs[remFile].ids.erase(id);
                if(_procs[remFile].ids.size() == 0) {
                    auto isFinished = _procs[remFile].proc->isFinished();
                    _procs[remFile].proc->close();
                    _procs.erase(remFile);
                    _fileCache->fileClosed(remFile, isFinished);
                }
            }
            if(_readers.find(remFile) != _readers.end()) {
                _readers[remFile].ids.erase(id);
                if(_readers[remFile].ids.size() == 0) {
                    _readers.erase(remFile);
                }
            }
        }
        uint_fast32_t getFileSize(const std::string &filename) override{
            RemoteFile remFile = _api->getRemoteFile(filename);
            return _fileCache->getFileSize(remFile);
        }
    private:
        std::shared_ptr<TVkApi> _api;
        std::shared_ptr<TFileCache> _fileCache;
        TFact _injector;

        struct ProcMapEntry{
            std::shared_ptr<TFileProcessor> proc;
            std::unordered_set<uint_fast32_t> ids;
        };
        std::unordered_map<RemoteFile, ProcMapEntry, RemoteFileHasher> _procs;

        struct ReadMapEntry{
            std::unordered_map<uint_fast32_t, std::variant<std::shared_ptr<TReader>, std::shared_ptr<TFileProcessor>>> ids;
            std::string fname;
        };
        std::unordered_map<RemoteFile, ReadMapEntry, RemoteFileHasher> _readers;

        std::unordered_map<uint_fast32_t, RemoteFile> _idToRemFile;

        std::mutex _procsMutex;
        std::mutex _readersMutex;
    };
}
