#include "BlockingBuffer.h"

using namespace vk_music_fs;

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
            uint_fast32_t start = std::min<uint_fast32_t>(offset, _buffer.size());
            uint_fast32_t end = std::min<uint_fast32_t>(offset + len, _buffer.size());
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

void BlockingBuffer::prepend(ByteVect vect, uint_fast32_t replace) {
    _startPart = std::move(vect);
    _prepBufSize = _startPart.size();
    _replaceLen = replace;
}

ByteVect BlockingBuffer::clearMain() {
    if(_replaceLen != 0){
        return ByteVect(_buffer.cbegin() + _replaceLen, _buffer.cend());
    } else {
        return std::move(_buffer);
    }
}

ByteVect BlockingBuffer::clearStart() {
    return std::move(_startPart);
}

BlockingBuffer::BlockingBuffer() : _eof(false), _portionRead(false), _replaceLen(0), _prepBufSize(0){
}

void BlockingBuffer::setEOF() {
    _eof = true;
    notify();
}

void BlockingBuffer::notify() {
    _portionRead = true;
    _portionReadCond.notify_one();
}

uint_fast32_t BlockingBuffer::getSize() {
    return _size;
}

uint_fast32_t BlockingBuffer::getPrependSize() {
    return _prepBufSize - _replaceLen;
}

void BlockingBuffer::setSize(uint_fast32_t size) {
    _size = size;
}
