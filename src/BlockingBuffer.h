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
        void prepend(ByteVect vect);
        ByteVect clearMain();
        ByteVect clearStart();
        uint_fast32_t getSize();
    private:
        void notify();

        std::atomic_bool _eof;
        std::atomic_uint_fast32_t _size;
        std::mutex _bufMutex;
        ByteVect _buffer;
        ByteVect _startPart;
        std::atomic_bool _portionRead;
        std::condition_variable _portionReadCond;
    };
}
