#include "FileProcessor.h"

using namespace vk_music_fs;

FileProcessorInt::FileProcessorInt()
:
    _buffer(std::make_shared<BlockingBuffer>()),
    _metadataWasRead(false), _closed(false), _finished(false),
    _opened(false), _error(false),
    _openedPromise(std::make_shared<std::promise<void>>()),
    _threadPromise(std::make_shared<std::promise<void>>()),
    _bufferAppendPromise(std::make_shared<std::promise<void>>()),
    _openFuture(std::move(_openedPromise->get_future())),
    _threadFinishedFuture(std::move(_threadPromise->get_future())),
    _bufferAppendFuture(std::move(_bufferAppendPromise->get_future())){
};

bool FileProcessorInt::addToBuffer(std::optional<ByteVect> vect) {
    if(vect) {
        _buffer->append(std::move(*vect));
        if(_metadataWasRead){
            notifyAppendStopped();
        }
        return _metadataWasRead;
    } else {
        _buffer->setEOF();
        notifyAppendStopped();
        return true;
    }
}

void FileProcessorInt::waitForStopAppend() {
    _metadataWasRead = true;
    std::unique_lock<std::mutex> lock(_bufferAppendMutex);
    _bufferAppendFuture.get();
}

void FileProcessorInt::notifyAppendStopped() {
    _bufferAppendPromise->set_value();
}
