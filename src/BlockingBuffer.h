#pragma once

#include <string>
#include <vector>
#include <memory>
#include <future>
#include <optional>
#include <common.h>

namespace vk_music_fs {
    class BlockingBuffer {
    public:
        explicit BlockingBuffer();
        void setSize(uint_fast32_t size);
        void setEOF();
        ByteVect read(uint_fast32_t offset, uint_fast32_t len);
        void append(ByteVect vect);
        void prepend(ByteVect vect, uint_fast32_t replace);
        ByteVect clearMain();
        ByteVect clearStart();
        uint_fast32_t getSize();
        uint_fast32_t getPrependSize();
    private:
        void notify();

        uint_fast32_t _replaceLen;
        uint_fast32_t _prepBufSize;
        std::atomic_bool _eof;
        std::atomic_uint_fast32_t _size;
        std::mutex _bufMutex;
        ByteVect _buffer;
        ByteVect _startPart;
        std::atomic_bool _portionRead;
        std::condition_variable _portionReadCond;
    };
}
