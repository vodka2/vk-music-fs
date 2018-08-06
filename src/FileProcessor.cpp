#include "FileProcessor.h"

using namespace vk_music_fs;

FileProcessorInt::FileProcessorInt(uint_fast32_t size)
: _buffer(std::make_shared<BlockingBuffer>(size)), _metadataWasRead(false), _bufferAppendStopped(false){
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

ByteVect BlockingBuffer::read(uint_fast32_t offset, uint_fast32_t len) {
    std::unique_lock<std::mutex> lock(_bufMutex);
    while(true) {
        if(!_eof) {
            while(!_portionRead) {
                _portionReadCond.wait(lock);
            }
            _portionRead = false;
        }
        if(_buffer.size() >= offset + len || _eof){
            ByteVect res;
            uint_fast32_t start = std::min(offset, _buffer.size());
            uint_fast32_t end = std::min(offset + len, _buffer.size());
            if(start != _buffer.size()) {
                res.reserve(end - start);
                std::copy(_buffer.cbegin() + start, _buffer.cbegin() + end, std::back_inserter(res));
            }
            return std::move(res);
        }
    }
}

void BlockingBuffer::append(ByteVect vect) {
    {
        std::scoped_lock<std::mutex> lock(_bufMutex);
        std::copy(vect.cbegin(), vect.cend(), std::back_inserter(_buffer));
    }
    notify();
}

void BlockingBuffer::prepend(ByteVect vect) {
    _startPart = std::move(vect);
}

ByteVect BlockingBuffer::clearMain() {
    return std::move(_buffer);
}

ByteVect BlockingBuffer::clearStart() {
    return std::move(_startPart);
}

BlockingBuffer::BlockingBuffer(uint_fast32_t size) : _eof(false), _size(size), _portionRead(false){
}

void BlockingBuffer::setEOF() {
    _eof = true;
    notify();
}

void BlockingBuffer::notify() {
    _portionRead = true;
    _portionReadCond.notify_one();
}
