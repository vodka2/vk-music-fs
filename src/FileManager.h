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
#include "net/HttpException.h"
#include <mutex>

namespace vk_music_fs {

    template <typename TAudioFs, typename TFileCache, typename TFileProcessor, typename TReader>
    class FileManager{
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

        int_fast32_t open(const std::string &filename){
            namespace di = boost::di;
            std::scoped_lock<std::mutex> readersLock(_readersMutex);
            std::scoped_lock<std::mutex> procsLock(_procsMutex);
            uint_fast32_t retId = _idToRemFile.size() + 1;
            try{
                RemoteFile remFile = _audioFs->getRemoteFile(filename);
                auto remFileId = remFile.getId();
                _idToRemFile.insert(std::make_pair<>(retId, remFile));
                if(_procs.find(remFileId) != _procs.end()){
                    _procs[remFileId].ids.insert(retId);
                    _readers[remFileId].ids.insert(std::make_pair<>(retId, _procs[remFileId].proc));
                } else {
                    auto fname = _fileCache->getFilename(remFile);
                    if(fname.inCache){
                        std::shared_ptr<TReader> reader = _readersFact->createShared(
                                CachedFilename{fname.data}, FileSize{_fileCache->getFileSize(remFile)}
                        );
                        _readers[remFileId].ids.insert(std::make_pair<>(retId, reader));
                    } else {
                        _procs[remFileId].proc = _procsFact->createShared(
                                Artist{remFile.getArtist()},
                                Title{remFile.getTitle()},
                                Mp3Uri{remFile.getUri()},
                                TagSize{_fileCache->getTagSize(remFile)},
                                RemoteFile(remFile),
                                CachedFilename{fname.data}
                        );
                        _readers[remFileId].ids.insert(std::make_pair<>(retId, _procs[remFileId].proc));
                        _procs[remFileId].ids.insert(retId);
                    }
                    _readers[remFileId].fname = fname.data;
                }
            } catch (const net::HttpException &ex){
                closeNoLock(retId);
                throw RemoteException("Error opening file " + filename + ". " + ex.what());
            }
            return retId;
        }
        ByteVect read(uint_fast32_t id, uint_fast32_t offset, uint_fast32_t size){
            ByteVect ret;
            std::scoped_lock<std::mutex> readersLock(_readersMutex);
            if(_idToRemFile.find(id) != _idToRemFile.end()) {
                try {
                    auto reader = _readers[_idToRemFile.find(id)->second.getId()].ids.find(id)->second;
                    std::visit([id, offset, size, &ret](auto &&el) {
                        ret = std::move(el->read(offset, size));
                    }, reader);
                    return std::move(ret);
                } catch (const net::HttpException &ex){
                    auto fname = _readers[_idToRemFile.find(id)->second.getId()].fname;
                    closeNoLock(id);
                    throw RemoteException("Error reading from " + fname + ". " + ex.what());
                } catch (const RemoteException &ex){
                    closeNoLock(id);
                    throw;
                }
            }
            return {};
        }
        void close(uint_fast32_t id){
            std::scoped_lock<std::mutex> readersLock(_readersMutex);
            std::scoped_lock<std::mutex> procsLock(_procsMutex);
            closeNoLock(id);
        }

        uint_fast32_t getFileSize(const std::string &filename){
            RemoteFile remFile = _audioFs->getRemoteFile(filename);
            try {
                return _fileCache->getFileSize(remFile);
            } catch (const net::HttpException &ex){
                throw RemoteException("Error getting file size " + filename + ". " + ex.what());
            }
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

        void closeNoLock(uint_fast32_t id){
            if(_idToRemFile.find(id) != _idToRemFile.end()) {
                auto remFile = _idToRemFile.find(id)->second;
                auto remFileId = remFile.getId();
                if (_procs.find(remFileId) != _procs.end()) {
                    _procs[remFileId].ids.erase(id);
                    if (_procs[remFileId].ids.size() == 0) {
                        _procs[remFileId].proc->close();
                        _procs.erase(remFileId);
                    }
                }
                if (_readers.find(remFileId) != _readers.end()) {
                    _readers[remFileId].ids.erase(id);
                    if (_readers[remFileId].ids.size() == 0) {
                        _readers.erase(remFileId);
                    }
                }
                _idToRemFile.erase(id);
            }
        }

        struct ProcMapEntry{
            std::shared_ptr<TFileProcessor> proc;
            std::unordered_set<uint_fast32_t> ids;
        };
        std::unordered_map<RemoteFileId, ProcMapEntry, RemoteFileIdHasher> _procs;

        struct ReadMapEntry{
            std::unordered_map<uint_fast32_t, std::variant<std::shared_ptr<TReader>, std::shared_ptr<TFileProcessor>>> ids;
            std::string fname;
        };
        std::unordered_map<RemoteFileId, ReadMapEntry, RemoteFileIdHasher> _readers;

        std::unordered_map<uint_fast32_t, RemoteFile> _idToRemFile;

        std::mutex _procsMutex;
        std::mutex _readersMutex;
    };
}
