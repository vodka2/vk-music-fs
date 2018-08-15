#pragma once

#include <string>
#include <vector>
#include <memory>
#include <future>
#include <optional>
#include <common.h>
#include <BlockingBuffer.h>

namespace vk_music_fs {
    class FileProcessorInt{
    protected:
        explicit FileProcessorInt();
        bool addToBuffer(std::optional<ByteVect> vect);
        void waitForStopAppend();
        void notifyAppendStopped();
        std::shared_ptr<BlockingBuffer> _buffer;
        std::atomic_bool _metadataWasRead;
        std::mutex _bufferAppendMutex;
        std::condition_variable _bufferAppendStoppedCond;
        std::atomic_bool _bufferAppendStopped;
        std::atomic_bool _closed;
        std::atomic_bool _finished;
        std::atomic_bool _opened;
        std::shared_ptr<std::promise<void>> _openedPromise;
        std::shared_ptr<std::promise<void>> _threadPromise;
        uint_fast32_t _prependSize;
        std::future<void> _openFuture;
        std::future<void> _threadFinishedFuture;
    };

    template <typename TStream, typename TFile, typename TMp3Parser, typename TPool>
    class FileProcessor : private FileProcessorInt{
    public:
        FileProcessor(
                const std::shared_ptr<TStream> &stream,
                const std::shared_ptr<TFile> &file,
                const std::shared_ptr<TPool> &pool,
                const std::shared_ptr<TMp3Parser> &parser
        )
        :FileProcessorInt(), _stream(stream), _file(file), _pool(pool), _parser(parser){
        }

        bool isFinished(){
            return _finished;
        }

        ByteVect read(uint_fast32_t start, uint_fast32_t size){
            bool exp = false;
            if(_opened.compare_exchange_strong(exp, true)){
                _pool->post([this] {
                    _stream->open(
                            _file->getSize() - _file->getPrependSize(),
                            _file->getTotalSize() - _file->getPrependSize()
                    );
                    _file->open();
                    if(_file->getSize() == 0) {
                        _buffer->setSize(_file->getTotalSize() - _file->getPrependSize());
                        _pool->post([this] {
                            while (!addToBuffer(_stream->read())) {
                                std::this_thread::sleep_for(std::chrono::milliseconds(30));
                            }
                        });
                        _parser->parse(_buffer);
                        waitForStopAppend();
                        _prependSize = _buffer->getPrependSize();
                        _file->setPrependSize(_prependSize);
                        _file->write(std::move(_buffer->clearStart()));
                        _file->write(std::move(_buffer->clearMain()));
                        _buffer.reset();
                    } else {
                        _prependSize = _file->getPrependSize();
                    }
                    _openedPromise->set_value();
                    while(true){
                        if(_closed){
                            _stream->close();
                            break;
                        }
                        auto buf = _stream->read();
                        if(!buf){
                            _finished = true;
                            _file->finish();
                            break;
                        }
                        _file->write(std::move(*buf));
                    }
                    _threadPromise->set_value();
                });
            }

            _openFuture.wait();
            auto fileSize = _file->getSize();
            if(start + size <= fileSize){
                return _file->read(start, size);
            } else {
                if(start < fileSize){
                    ByteVect res = std::move(_file->read(start, (fileSize - start)));
                    res.reserve(size);
                    ByteVect t = std::move(_stream->read(fileSize - _prependSize, size - (fileSize - start)));
                    std::copy(t.cbegin(), t.cend(), std::back_inserter(res));
                    return std::move(res);
                } else {
                    return _stream->read(start - _prependSize, size);
                }
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
                _threadFinishedFuture.wait();
            }
        }
    private:
        std::shared_ptr<TStream> _stream;
        std::shared_ptr<TFile> _file;
        std::shared_ptr<TPool> _pool;
        std::shared_ptr<TMp3Parser> _parser;
    };
}