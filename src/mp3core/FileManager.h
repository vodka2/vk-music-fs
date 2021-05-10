#pragma once

#include <common/common.h>
#include "RemoteException.h"
#include <memory>
#include <boost/di.hpp>
#include <unordered_set>
#include <unordered_map>
#include <variant>
#include <diext/ext_factory.hpp>
#include "net/HttpException.h"
#include <mutex>
#include <net/WrongSizeException.h>
#include <atomic>
#include <common/RemotePhotoFile.h>
#include <common/IdGenerator.h>

namespace vk_music_fs {
    template <
            typename TFileCache, typename TReader, typename TFileProcessorFact, typename TItem
            >
    class FileManager{
    public:
        typedef typename TItem::IdType TId;
        typedef typename TId::Hasher THasher;
        typedef typename TFileProcessorFact::Processor TFileProcessor;
        FileManager(
                const std::shared_ptr<TFileCache> &fileCache,
                std::shared_ptr<TFileProcessorFact> procsFact,
                std::shared_ptr<boost::di::extension::iextfactory<
                        TReader,
                        CachedFilename,
                        FileSize
                >> readersFact,
                std::shared_ptr<ManagerIdGenerator> idGenerator
        ) : _fileCache(fileCache), _managerIdGenerator(idGenerator),
        _procsFact(procsFact), _readersFact(readersFact) {
        }

        int_fast32_t open(const TItem &remFile, const std::string &filename){
            namespace di = boost::di;
            std::scoped_lock<std::mutex> readersLock(_readersMutex);
            uint_fast32_t retId = _managerIdGenerator->getNextId();
            try{
                auto remFileId = remFile.getId();
                _idToRemFile.insert(std::make_pair<>(retId, remFile));
                try {
                    auto fname = _fileCache->getFilename(remFile);
                    if (fname.inCache) {
                        _mp3Readers[remFileId].processor = std::nullopt;
                        std::shared_ptr<TReader> reader = _readersFact->createShared(
                                CachedFilename{fname.data}, FileSize{_fileCache->getFileSize(remFile)}
                        );
                        _mp3Readers[remFileId].ids.insert(std::make_pair<>(retId, reader));
                    } else {
                        if (_mp3Readers[remFileId].processor) {
                            _mp3Readers[remFileId].ids.insert(std::make_pair<>(retId, *_mp3Readers[remFileId].processor));
                            _mp3Readers[remFileId].usages[*_mp3Readers[remFileId].processor]++;
                        } else {
                            auto proc = _procsFact->create(remFile, _fileCache, CachedFilename{fname.data});
                            _mp3Readers[remFileId].ids.insert(std::make_pair<>(retId, proc));
                            _mp3Readers[remFileId].processor = proc;
                            _mp3Readers[remFileId].usages[proc]++;
                        }
                    }
                    _mp3Readers[remFileId].fname = fname.data;
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

        bool ownsFile(uint_fast32_t id) {
            std::scoped_lock<std::mutex> readersLock(_readersMutex);
            return _idToRemFile.find(id) != _idToRemFile.end();
        }

        ByteVect read(uint_fast32_t id, uint_fast32_t offset, uint_fast32_t size){
            ByteVect ret;
            std::string fname;
            std::optional<TId> remoteFileId;
            std::variant<std::shared_ptr<TReader>, std::shared_ptr<TFileProcessor>> reader;
            {
                std::scoped_lock<std::mutex> readersLock(_readersMutex);
                if(_idToRemFile.find(id) != _idToRemFile.end()) {
                    remoteFileId = _idToRemFile.find(id)->second.getId();
                    fname = _mp3Readers[*remoteFileId].fname;
                    reader = _mp3Readers[*remoteFileId].ids.find(id)->second;
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

        uint_fast32_t getFileSize(const TItem &remFile, const std::string &filename){
            try {
                return _fileCache->getFileSize(remFile);
            } catch (const net::HttpException &ex){
                throw RemoteException("Error getting file size " + filename + ". " + ex.what());
            }
        }
    private:
        std::shared_ptr<TFileCache> _fileCache;
        std::shared_ptr<TFileProcessorFact> _procsFact;
        std::shared_ptr<ManagerIdGenerator> _managerIdGenerator;

        std::shared_ptr<boost::di::extension::iextfactory<
                TReader,
                CachedFilename,
                FileSize
        >> _readersFact;

        void closeNoLock(uint_fast32_t id){
            if(_idToRemFile.find(id) != _idToRemFile.end()) {
                auto remFile = _idToRemFile.find(id)->second;
                auto remFileId = remFile.getId();
                if (_mp3Readers.find(remFileId) != _mp3Readers.end()) {
                    auto reader = _mp3Readers[remFileId].ids.find(id)->second;
                    if (_mp3Readers[remFileId].usages[reader] <= 1) {
                        std::visit([](auto &&el) {
                            el->close();
                        }, reader);
                    } else {
                        _mp3Readers[remFileId].usages[reader]--;
                    }
                    _mp3Readers[remFileId].ids.erase(id);
                    if (_mp3Readers[remFileId].ids.size() == 0) {
                        _mp3Readers.erase(remFileId);
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
        std::unordered_map<TId, ReadMapEntry, THasher> _mp3Readers;

        std::unordered_map<uint_fast32_t, TItem> _idToRemFile;

        std::mutex _readersMutex;
    };
}
