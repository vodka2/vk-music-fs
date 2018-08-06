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
        explicit FileProcessorInt(uint_fast32_t size);
        bool addToBuffer(std::optional<ByteVect> vect);
        void waitForStopAppend();
        void notifyAppendStopped();
        std::shared_ptr<BlockingBuffer> _buffer;
        std::atomic_bool _metadataWasRead;
        std::mutex _bufferAppendMutex;
        std::condition_variable _bufferAppendStoppedCond;
        std::atomic_bool _bufferAppendStopped;
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
        :FileProcessorInt(stream->getSize()), _stream(stream), _file(file), _pool(pool), _parser(parser){
            auto promise = std::make_shared<std::promise<void>>();
            _pool->post([this, promise] {
                _pool->post([this, promise] {
                    while(!addToBuffer(_stream->read()));
                });
                _parser->parse(_buffer);
                waitForStopAppend();
                _file->write(std::move(_buffer->clearStart()));
                _file->write(std::move(_buffer->clearMain()));
                promise->set_value();
            });
            _openFuture = std::move(promise->get_future());
        }

        std::future<void>& openFile(){
            return _openFuture;
        }
    private:
        std::future<void> _openFuture;

        std::shared_ptr<TStream> _stream;
        std::shared_ptr<TFile> _file;
        std::shared_ptr<TPool> _pool;
        std::shared_ptr<TMp3Parser> _parser;
    };
}