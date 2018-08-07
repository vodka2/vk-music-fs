#include "FileProcessor.h"

using namespace vk_music_fs;

FileProcessorInt::FileProcessorInt(uint_fast32_t size)
:
    _buffer(std::make_shared<BlockingBuffer>(size)),
    _metadataWasRead(false), _bufferAppendStopped(false), _closed(false){
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
    while(!_bufferAppendStopped) {
        _bufferAppendStoppedCond.wait(lock);
    }
}

void FileProcessorInt::notifyAppendStopped() {
    _bufferAppendStopped = true;
    _bufferAppendStoppedCond.notify_one();
}
