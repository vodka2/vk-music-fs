#pragma once
#include <common/common.h>
#include <net/WrongSizeException.h>
#include "RemoteException.h"

namespace vk_music_fs {
    template <typename TStream, typename TFile, typename TBlockCreator>
    class PhotoProcessor {
    public:
        PhotoProcessor(
                const std::shared_ptr<TStream> &stream, const std::shared_ptr<TFile> &file,
                const std::shared_ptr<TBlockCreator> &blockCreator):
                _stream(stream), _file(file), _blockCreator(blockCreator) {}

        ByteVect read(uint_fast32_t start, uint_fast32_t size) {
            std::unique_lock<std::mutex> lock(_readMutex);
            _file->open();
            if (_file->getUriSize() != _file->getSizeOnDisk()) {
                try {
                    _stream->open(_file->getSizeOnDisk(), _file->getUriSize());
                    auto block = _blockCreator->create();
                    while (true) {
                        _stream->read(block);
                        if (block->curSize() == 0) {
                            break;
                        }
                        _file->write(block);
                        block->reset();
                    }
                    _stream->close();
                } catch (const net::WrongSizeException &ex){
                    _stream->close();
                    close();
                    throw;
                } catch (const MusicFsException &ex){
                    _stream->close();
                    close();
                    throw RemoteException(std::string("Error reading remote file. ") + ex.what());
                }
            }
            return _file->read(start, size);
        }

        void close() {
            _file->close();
        }

    private:
        std::mutex _readMutex;
        std::shared_ptr<TStream> _stream;
        std::shared_ptr<TFile> _file;
        std::shared_ptr<TBlockCreator> _blockCreator;
    };
}