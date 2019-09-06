#pragma once

#include <string>
#include <vector>
#include <memory>
#include <future>
#include <optional>
#include <common/IOBlock.h>
#include "common_mp3core.h"

namespace vk_music_fs {
    class BlockingBufferInt {
    public:
        BlockingBufferInt();
        void setSize(uint_fast32_t size);
        void setEOF();
        void prepend(ByteVect vect, uint_fast32_t replace);
        std::shared_ptr<IOBlockWrap<IOBlockVect>> clearStart();
        uint_fast32_t getSize();
        uint_fast32_t getPrependSize();

    protected:
        void waitForPortionRead(std::unique_lock<std::mutex> &lock);
        void notify();

        std::atomic_bool _sizeLimitReached;
        uint_fast32_t _replaceLen;
        uint_fast32_t _prepBufSize;
        std::atomic_bool _eof;
        std::atomic_uint_fast32_t _size;
        std::mutex _bufMutex;
        ByteVect _startPart;
        std::atomic_bool _portionRead;
        std::condition_variable _portionReadCond;
        uint_fast32_t _maxBufferSize;
    };

    template <typename TBlockCreator, typename TFileBuffer>
    class BlockingBuffer : public BlockingBufferInt{
    public:
        explicit BlockingBuffer(const std::shared_ptr<TBlockCreator> &blockCreator, const std::shared_ptr<TFileBuffer> &fileBuffer)
            : BlockingBufferInt(), _memoryBuffer(blockCreator->create()), _fileBuffer(fileBuffer) {
            _maxBufferSize = _memoryBuffer->maxSize();
        }
        ByteVect read(uint_fast32_t offset, uint_fast32_t len) {
            std::unique_lock<std::mutex> lock(_bufMutex);
            IOBlockWrap<IOBlockVect> block(IOBlockVect{len});
            while (true) {
                waitForPortionRead(lock);
                if (readNoLock(offset + block.curSize(), &block)) {
                    block.arr().resize(block.curSize());
                    return std::move(block.arr());
                }
            }
        }

        template <typename TBlock>
        void append(TBlock block) {
            {
                std::scoped_lock<std::mutex> lock(_bufMutex);
                if (_sizeLimitReached || ((_memoryBuffer->curSize() + block->curSize()) > _memoryBuffer->maxSize())) {
                    _sizeLimitReached = true;
                    _fileBuffer->write(block);
                } else {
                    std::copy(block->addr(), block->addrCurSize(), _memoryBuffer->addrCurSize());
                    _memoryBuffer->curSize() += block->curSize();
                }
            }
            notify();
        }

        template <typename TBlock>
        void finalRead(uint_fast32_t offset, TBlock block) {
            offset += _replaceLen;
            readNoLock(offset, block);
        }

    private:
        template <typename TBlock>
        bool readNoLock(uint_fast32_t offset, TBlock block) {
            uint_fast32_t targetLen = block->maxSize() - block->curSize();

            if (_memoryBuffer->curSize() > offset) {
                uint_fast32_t start = offset;
                uint_fast32_t end = std::min<uint_fast32_t>(offset + targetLen, _memoryBuffer->curSize());
                std::copy(_memoryBuffer->addr(start), _memoryBuffer->addr(end), block->addrCurSize());
                block->curSize() += end - start;
            }

            if (block->curSize() != targetLen && _sizeLimitReached) {
                offset = offset + block->curSize() - _memoryBuffer->curSize();
                _fileBuffer->read(offset, block);
            }

            return block->curSize() == block->maxSize() || _eof;
        }

        typename TBlockCreator::BlockPtr _memoryBuffer;
        std::shared_ptr<TFileBuffer> _fileBuffer;
    };
}
