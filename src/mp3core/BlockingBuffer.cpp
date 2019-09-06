#include "BlockingBuffer.h"

using namespace vk_music_fs;

void BlockingBufferInt::prepend(ByteVect vect, uint_fast32_t replace) {
    _startPart = std::move(vect);
    _prepBufSize = _startPart.size();
    _replaceLen = replace;
}

std::shared_ptr<IOBlockWrap<IOBlockVect>> BlockingBufferInt::clearStart() {
    return std::make_shared<IOBlockWrap<IOBlockVect>>(IOBlockWrap{IOBlockVect{std::move(_startPart)}});
}

BlockingBufferInt::BlockingBufferInt()
: _sizeLimitReached(false), _eof(false), _portionRead(false), _replaceLen(0), _prepBufSize(0){
}

void BlockingBufferInt::setEOF() {
    _eof = true;
    notify();
}

void BlockingBufferInt::notify() {
    _portionRead = true;
    _portionReadCond.notify_one();
}

uint_fast32_t BlockingBufferInt::getSize() {
    return _size;
}

uint_fast32_t BlockingBufferInt::getPrependSize() {
    return _prepBufSize - _replaceLen;
}

void BlockingBufferInt::setSize(uint_fast32_t size) {
    _size = size;
}

void BlockingBufferInt::waitForPortionRead(std::unique_lock<std::mutex> &lock) {
    if(!_eof) {
        while(!_portionRead) {
            _portionReadCond.wait(lock);
        }
        _portionRead = false;
    }
}
