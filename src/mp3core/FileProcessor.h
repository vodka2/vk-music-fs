#pragma once

#include <string>
#include <vector>
#include <memory>
#include <future>
#include <optional>
#include <common/common.h>
#include "BlockingBuffer.h"
#include "RemoteException.h"
#include "net/WrongSizeException.h"

namespace vk_music_fs {
    class FileProcessorInt{
    public:
        bool isFinished();
    protected:
        explicit FileProcessorInt();
        void waitForStopAppend();
        void notifyAppendStopped();
        std::atomic_bool _metadataWasRead;
        std::atomic_bool _bufferSizeLimitReached;
        std::mutex _bufferAppendMutex;
        std::atomic_bool _closed;
        std::atomic_bool _finished;
        std::atomic_bool _opened;
        std::atomic_bool _error;
        std::shared_ptr<std::promise<void>> _openedPromise;
        std::shared_ptr<std::promise<void>> _threadPromise;
        std::shared_ptr<std::promise<void>> _bufferAppendPromise;
        uint_fast32_t _prependSize;
        std::shared_future<void> _openFuture;
        std::shared_future<void> _threadFinishedFuture;
        std::shared_future<void> _bufferAppendFuture;
    };

    template <typename TStream, typename TFile, typename TMp3Parser, typename TPool, typename TBuffer, typename TBlockCreator>
    class FileProcessor : public FileProcessorInt{
    public:
        FileProcessor(
                const std::shared_ptr<TStream> &stream,
                const std::shared_ptr<TFile> &file,
                const std::shared_ptr<TPool> &pool,
                const std::shared_ptr<TMp3Parser> &parser,
                const std::shared_ptr<TBuffer> &buffer,
                const std::shared_ptr<TBlockCreator> &blockCreator
        )
        :FileProcessorInt(), _stream(stream), _file(file), _pool(pool), _parser(parser), _buffer(buffer),
        _blockCreator(blockCreator){
        }

        template <typename T>
        bool addToBuffer(T block) {
            if (block->curSize() != 0) {
                _buffer->append(block);
                if (_metadataWasRead) {
                    notifyAppendStopped();
                }
                return _metadataWasRead;
            } else {
                _buffer->setEOF();
                notifyAppendStopped();
                return true;
            }
        }

        ByteVect read(uint_fast32_t start, uint_fast32_t size){
            bool exp = false;
            if(_opened.compare_exchange_strong(exp, true)){
                _pool->post([this] {
                    try {
                        try {
                            _stream->open(
                                    _file->getSizeOnDisk() - _file->getPrependSize(),
                                    _file->getUriSize()
                            );
                            _file->open();
                            if (_file->getSizeOnDisk() == 0) {
                                _buffer->setSize(_file->getUriSize());
                                _pool->post([this] {
                                    try {
                                        auto block = _blockCreator->create();
                                        while (true) {
                                            _stream->read(block);
                                            if (addToBuffer(block)) {
                                                break;
                                            }
                                            block->reset();
                                            std::this_thread::sleep_for(std::chrono::milliseconds(30));
                                        }
                                    } catch (const MusicFsException &ex) {
                                        _buffer->setEOF();
                                        _bufferAppendPromise->set_exception(std::current_exception());
                                    }
                                });
                                _parser->parse(_buffer);
                                waitForStopAppend();
                                _prependSize = _buffer->getPrependSize();
                                _file->setPrependSize(_prependSize);
                                _file->write(std::move(_buffer->clearStart()));

                                auto block = _blockCreator->create();
                                uint_fast32_t offset = 0;
                                while (true) {
                                    _buffer->finalRead(offset, block);
                                    if (block->curSize() == 0) {
                                        break;
                                    }
                                    offset += block->curSize();
                                    _file->write(block);
                                    block->reset();
                                }
                                _buffer.reset();
                            } else {
                                _prependSize = _file->getPrependSize();
                            }
                        } catch (const MusicFsException &ex) {
                            _openedPromise->set_exception(std::current_exception());
                            throw;
                        }
                        _openedPromise->set_value();
                        auto block = _blockCreator->create();
                        while (true) {
                            if (_closed) {
                                _stream->close();
                                break;
                            }
                            _stream->read(block);
                            if (block->curSize() == 0) {
                                _finished = true;
                                _file->finish();
                                break;
                            }
                            _file->write(block);
                            block->reset();
                        }
                        _threadPromise->set_value();
                    } catch (const MusicFsException &ex){
                        _error = true;
                        _threadPromise->set_exception(std::current_exception());
                    }
                });
            }
            try {
                _openFuture.get();
                if (_error) {
                    _threadFinishedFuture.get();
                }
                auto fileSize = _file->getSizeOnDisk();
                if (start + size <= fileSize) {
                    return _file->read(start, size);
                } else {
                    if (start < fileSize) {
                        ByteVect res = std::move(_file->read(start, (fileSize - start)));
                        res.reserve(size);
                        ByteVect t = std::move(_stream->read(fileSize - _prependSize, size - (fileSize - start)));
                        std::copy(t.cbegin(), t.cend(), std::back_inserter(res));
                        return std::move(res);
                    } else {
                        return _stream->read(start - _prependSize, size);
                    }
                }
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

        void close(){
            _file->close();
            _closed = true;
        }

        ~FileProcessor() {
            if (!_closed) {
                close();
            }
            if (_opened) {
                try {
                    _threadFinishedFuture.get();
                } catch (const MusicFsException &ex){
                }
            }
        }
    private:
        std::shared_ptr<TStream> _stream;
        std::shared_ptr<TFile> _file;
        std::shared_ptr<TPool> _pool;
        std::shared_ptr<TMp3Parser> _parser;
        std::shared_ptr<TBuffer> _buffer;
        std::shared_ptr<TBlockCreator> _blockCreator;
    };
}