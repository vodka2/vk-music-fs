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
            auto openPromise = std::make_shared<std::promise<void>>();
            auto threadPromise = std::make_shared<std::promise<void>>();
            _openFuture = std::move(openPromise->get_future());
            _threadFinishedFuture = std::move(threadPromise->get_future());
            _pool->post([this, openPromise, threadPromise] {
                _stream->open();
                _buffer->setSize(_stream->getSize());
                _pool->post([this, openPromise] {
                    while(!addToBuffer(_stream->read())){
                        std::this_thread::sleep_for(std::chrono::milliseconds(30));
                    }
                });
                _parser->parse(_buffer);
                waitForStopAppend();
                auto prepVect = std::move(_buffer->clearStart());
                _prependSize = prepVect.size();
                openPromise->set_value();
                _file->write(std::move(prepVect));
                _file->write(std::move(_buffer->clearMain()));
                _buffer.reset();
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
                threadPromise->set_value();
            });
        }

        std::future<void>& openFile(){
            return _openFuture;
        }

        void openBlocking(){
            _openFuture.wait();
        }

        bool isFinished(){
            return _finished;
        }

        ByteVect read(uint_fast32_t start, uint_fast32_t size){
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
            _closed = true;
            _file->close();
        }

        ~FileProcessor(){
            if(!_closed){
                _closed = true;
                _file->close();
            }
            _threadFinishedFuture.wait();
        }
    private:
        std::shared_ptr<TStream> _stream;
        std::shared_ptr<TFile> _file;
        std::shared_ptr<TPool> _pool;
        std::shared_ptr<TMp3Parser> _parser;
    };
}