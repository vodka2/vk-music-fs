#pragma once

#include <common.h>
#include <RemoteException.h>
#include <memory>
#include <boost/di.hpp>
#include <unordered_set>
#include <unordered_map>
#include <variant>
#include <ext_factory.hpp>
#include "RemoteFile.h"
#include "IFileManager.h"
#include <mutex>

namespace vk_music_fs {

    template <typename TAudioFs, typename TFileCache, typename TFileProcessor, typename TReader>
    class FileManager: public IFileManager {
    public:
        FileManager(
                const std::shared_ptr<TAudioFs> &audioFs,
                const std::shared_ptr<TFileCache> &fileCache,
                std::shared_ptr<boost::di::extension::iextfactory<
                        TFileProcessor,
                        Artist,
                        Title,
                        Mp3Uri,
                        TagSize,
                        RemoteFile,
                        CachedFilename
                >> procsFact,
                std::shared_ptr<boost::di::extension::iextfactory<
                        TReader,
                        CachedFilename,
                        FileSize
                >> readersFact
        ) :_audioFs(audioFs), _fileCache(fileCache),
        _procsFact(procsFact), _readersFact(readersFact){
        }

        int_fast32_t open(const std::string &filename) override{
            namespace di = boost::di;
            RemoteFile remFile = _audioFs->getRemoteFile(filename);
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
                    std::shared_ptr<TReader> reader = _readersFact->createShared(
                            CachedFilename{fname.data}, FileSize{_fileCache->getFileSize(remFile)}
                    );
                    _readers[remFile].ids.insert(std::make_pair<>(retId, reader));
                } else {
                    _procs[remFile].proc = _procsFact->createShared(
                            Artist{remFile.getArtist()},
                            Title{remFile.getTitle()},
                            Mp3Uri{remFile.getUri()},
                            TagSize{_fileCache->getTagSize(remFile)},
                            RemoteFile(remFile),
                            CachedFilename{fname.data}
                    );
                    _readers[remFile].ids.insert(std::make_pair<>(retId, _procs[remFile].proc));
                    _procs[remFile].ids.insert(retId);
                }
                _readers[remFile].fname = fname.data;
            }
            return retId;
        }
        ByteVect read(uint_fast32_t id, uint_fast32_t offset, uint_fast32_t size) override{
            ByteVect ret;
            _readersMutex.lock();
            if(_idToRemFile.find(id) != _idToRemFile.end()) {
                try {
                    std::variant < std::shared_ptr<TReader>, std::shared_ptr<TFileProcessor>>
                    reader;
                    reader = _readers[_idToRemFile.find(id)->second].ids.find(id)->second;
                    std::visit([id, offset, size, &ret](auto &&el) {
                        ret = std::move(el->read(offset, size));
                    }, reader);
                    _readersMutex.unlock();
                    return std::move(ret);
                } catch (const RemoteException &ex){
                    _readersMutex.unlock();
                    close(id);
                    throw;
                } catch (...){
                    _readersMutex.unlock();
                    throw;
                }
            }
            _readersMutex.unlock();
            return {};
        }
        void close(uint_fast32_t id) override{
            std::scoped_lock<std::mutex> procsLock(_procsMutex);
            std::scoped_lock<std::mutex> readersLock(_readersMutex);
            if(_idToRemFile.find(id) != _idToRemFile.end()) {
                auto remFile = _idToRemFile.find(id)->second;
                if (_procs.find(remFile) != _procs.end()) {
                    _procs[remFile].ids.erase(id);
                    if (_procs[remFile].ids.size() == 0) {
                        _procs[remFile].proc->close();
                        _procs.erase(remFile);
                    }
                }
                if (_readers.find(remFile) != _readers.end()) {
                    _readers[remFile].ids.erase(id);
                    if (_readers[remFile].ids.size() == 0) {
                        _readers.erase(remFile);
                    }
                }
                _idToRemFile.erase(id);
            }
        }
        uint_fast32_t getFileSize(const std::string &filename) override{
            RemoteFile remFile = _audioFs->getRemoteFile(filename);
            return _fileCache->getFileSize(remFile);
        }
    private:
        std::shared_ptr<TAudioFs> _audioFs;
        std::shared_ptr<TFileCache> _fileCache;
        std::shared_ptr<boost::di::extension::iextfactory<
                TFileProcessor,
                Artist,
                Title,
                Mp3Uri,
                TagSize,
                RemoteFile,
                CachedFilename
        >> _procsFact;

        std::shared_ptr<boost::di::extension::iextfactory<
                TReader,
                CachedFilename,
                FileSize
        >> _readersFact;

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
