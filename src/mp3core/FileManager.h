#pragma once

#include <common/common.h>
#include "RemoteException.h"
#include <memory>
#include <boost/di.hpp>
#include <unordered_set>
#include <unordered_map>
#include <variant>
#include <diext/ext_factory.hpp>
#include "RemoteFile.h"
#include "net/HttpException.h"
#include <mutex>
#include <net/WrongSizeException.h>
#include <atomic>

namespace vk_music_fs {

    template <typename TFileCache, typename TFileProcessor, typename TReader>
    class FileManager{
    public:
        FileManager(
                const std::shared_ptr<TFileCache> &fileCache,
                std::shared_ptr<boost::di::extension::iextfactory<
                        TFileProcessor,
                        SongData,
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
        ) : _fileCache(fileCache),
        _procsFact(procsFact), _readersFact(readersFact), _lastId(100){
        }

        int_fast32_t open(const RemoteFile &remFile, const std::string &filename){
            namespace di = boost::di;
            std::scoped_lock<std::mutex> readersLock(_readersMutex);
            uint_fast32_t retId = (_lastId++);
            try{
                auto remFileId = remFile.getId();
                _idToRemFile.insert(std::make_pair<>(retId, remFile));
                try {
                    auto fname = _fileCache->getFilename(remFile);
                    if (fname.inCache) {
                        _readers[remFileId].processor = std::nullopt;
                        std::shared_ptr<TReader> reader = _readersFact->createShared(
                                CachedFilename{fname.data}, FileSize{_fileCache->getFileSize(remFile)}
                        );
                        _readers[remFileId].ids.insert(std::make_pair<>(retId, reader));
                    } else {
                        if (_readers[remFileId].processor) {
                            _readers[remFileId].ids.insert(std::make_pair<>(retId, *_readers[remFileId].processor));
                            _readers[remFileId].usages[*_readers[remFileId].processor]++;
                        } else {
                            auto proc = _procsFact->createShared(
                                    SongData{remFile.getSongData()},
                                    Mp3Uri{remFile.getUri()},
                                    TagSize{_fileCache->getTagSize(remFile)},
                                    RemoteFile(remFile),
                                    CachedFilename{fname.data}
                            );
                            _readers[remFileId].ids.insert(std::make_pair<>(retId, proc));
                            _readers[remFileId].processor = proc;
                            _readers[remFileId].usages[proc]++;
                        }
                    }
                    _readers[remFileId].fname = fname.data;
                } catch (const net::WrongSizeException &ex) {
                    _fileCache->removeSize(remFileId);
                    closeNoLock(retId);
                    throw RemoteException("Error getting size from " + filename + ". " + ex.what());
                }
            } catch (const net::HttpException &ex){
                closeNoLock(retId);
                throw RemoteException("Error opening file " + filename + ". " + ex.what());
            }
            return retId;
        }
        ByteVect read(uint_fast32_t id, uint_fast32_t offset, uint_fast32_t size){
            ByteVect ret;
            std::string fname;
            std::optional<RemoteFileId> remoteFileId;
            std::variant<std::shared_ptr<TReader>, std::shared_ptr<TFileProcessor>> reader;
            {
                std::scoped_lock<std::mutex> readersLock(_readersMutex);
                if(_idToRemFile.find(id) != _idToRemFile.end()) {
                    remoteFileId = _idToRemFile.find(id)->second.getId();
                    fname = _readers[*remoteFileId].fname;
                    reader = _readers[*remoteFileId].ids.find(id)->second;
                }
            }
            if(remoteFileId) {
                try {
                    std::visit([id, offset, size, &ret](auto &&el) {
                        ret = std::move(el->read(offset, size));
                    }, reader);
                    return std::move(ret);
                } catch (const net::WrongSizeException &ex) {
                    _fileCache->removeSize(*remoteFileId);
                    close(id);
                    throw RemoteException("Error getting size from " + fname + ". " + ex.what());
                } catch (const net::HttpException &ex){
                    close(id);
                    throw RemoteException("Error reading from " + fname + ". " + ex.what());
                } catch (const RemoteException &ex){
                    close(id);
                    throw;
                }
            }
            return {};
        }
        void close(uint_fast32_t id){
            std::scoped_lock<std::mutex> readersLock(_readersMutex);
            closeNoLock(id);
        }

        uint_fast32_t getFileSize(const RemoteFile &remFile, const std::string &filename){
            try {
                return _fileCache->getFileSize(remFile);
            } catch (const net::HttpException &ex){
                throw RemoteException("Error getting file size " + filename + ". " + ex.what());
            }
        }
    private:
        std::shared_ptr<TFileCache> _fileCache;
        std::shared_ptr<boost::di::extension::iextfactory<
                TFileProcessor,
                SongData,
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
                if (_readers.find(remFileId) != _readers.end()) {
                    auto reader = _readers[remFileId].ids.find(id)->second;
                    if (_readers[remFileId].usages[reader] <= 1) {
                        std::visit([](auto &&el) {
                            el->close();
                        }, reader);
                    } else {
                        _readers[remFileId].usages[reader]--;
                    }
                    _readers[remFileId].ids.erase(id);
                    if (_readers[remFileId].ids.size() == 0) {
                        _readers.erase(remFileId);
                    }
                }
                _idToRemFile.erase(id);
            }
        }

        struct ReadMapEntry{
            std::optional<std::shared_ptr<TFileProcessor>> processor;
            std::unordered_map<std::variant<std::shared_ptr<TReader>, std::shared_ptr<TFileProcessor>>, uint_fast32_t> usages;
            std::unordered_map<uint_fast32_t, std::variant<std::shared_ptr<TReader>, std::shared_ptr<TFileProcessor>>> ids;
            std::string fname;
        };
        std::unordered_map<RemoteFileId, ReadMapEntry, RemoteFileIdHasher> _readers;

        std::unordered_map<uint_fast32_t, RemoteFile> _idToRemFile;

        std::atomic_uint_fast32_t _lastId;

        std::mutex _readersMutex;
    };
}
