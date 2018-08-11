#pragma once

#include <common.h>
#include <memory>
#include <boost/di.hpp>
#include <unordered_set>
#include <unordered_map>
#include <variant>
#include <boost/di/extension/injections/extensible_injector.hpp>
#include "RemoteFile.h"
#include <mutex>

namespace vk_music_fs {
    template <typename TVkApi, typename TFileCache, typename TFileProcessor, typename TReader>
    class FileManager {
    public:
        FileManager(
                const std::shared_ptr<TVkApi> &api,
                const std::shared_ptr<TFileCache> &fileCache,
                const InjPtr<
                        std::shared_ptr<TFileProcessor>,
                        std::shared_ptr<TReader>
                > &injector
        ) :_api(api), _fileCache(fileCache),
        _injector(injector){
        }
        int_fast32_t open(const std::string &filename){
            namespace di = boost::di;
            RemoteFile remFile = _api->getRemoteFile(filename);
            uint_fast32_t retId = _idToRemFile.size();
            _idToRemFile.insert(std::make_pair<>(retId, remFile));
            std::scoped_lock<std::mutex> procsLock(_procsMutex);
            std::scoped_lock<std::mutex> readersLock(_readersMutex);
            if(_procs.find(remFile) != _procs.end()){
                _procs[remFile].ids.insert(retId);
            } else {
                auto fname = _fileCache->getFilename(remFile);
                if(fname.inCache){
                    auto reader = di::make_injector(
                            di::extension::make_extensible(*_injector),
                            di::bind<CachedFilename>.to(CachedFilename{fname.data})
                    ).template create<std::shared_ptr<TReader>>();
                    _readers[remFile].ids.insert(std::make_pair<>(retId, reader));
                } else {
                    _procs[remFile].proc = di::make_injector(
                            di::extension::make_extensible(*_injector),
                            di::bind<Artist>.to(Artist{remFile.getArtist()}),
                            di::bind<Title>.to(Title{remFile.getTitle()}),
                            di::bind<Mp3Uri>.to(Mp3Uri{remFile.getUri()}),
                            di::bind<TagSize>.to(TagSize(_fileCache->getTagSize(remFile)))
                    ).template create<std::shared_ptr<TFileProcessor>>();
                    _readers[remFile].ids.insert(std::make_pair<>(retId, _procs[remFile].proc));
                    _procs[remFile].ids.insert(retId);
                }
                _readers[remFile].fname = fname.data;
            }
            return retId;
        }
        ByteVect read(uint_fast32_t id, uint_fast32_t offset, uint_fast32_t size){
            ByteVect ret;
            std::variant<std::shared_ptr<TReader>, std::shared_ptr<TFileProcessor>> reader;
            {
                std::scoped_lock<std::mutex> _lock(_readersMutex);
                reader = _readers[_idToRemFile.find(id)->second].ids.find(id)->second;
            }
            std::visit([id, offset, size, &ret](auto &&el) {
                el->openBlocking();
                ret = std::move(el->read(offset, size));
            }, reader);
            return std::move(ret);
        }
        void close(uint_fast32_t id){
            std::scoped_lock<std::mutex> procsLock(_procsMutex);
            std::scoped_lock<std::mutex> readersLock(_readersMutex);
            auto remFile = _idToRemFile.find(id)->second;
            _fileCache->fileClosed(_readers[remFile].fname);
            if(_readers.find(remFile) != _readers.end()) {
                _readers.erase(remFile);
            }
            if(_procs.find(remFile) != _procs.end()) {
                _procs.erase(remFile);
            }
        }
        uint_fast32_t getFileSize(const std::string &filename){
            RemoteFile remFile = _api->getRemoteFile(filename);
            return _fileCache->getFileSize(remFile);
        }
    private:
        std::shared_ptr<TVkApi> _api;
        std::shared_ptr<TFileCache> _fileCache;
        InjPtr<
                std::shared_ptr<TFileProcessor>,
                std::shared_ptr<TReader>
        > _injector;

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
