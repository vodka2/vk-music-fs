#pragma once
#include "common.h"
#include <array>

namespace vk_music_fs {
    template <uint32_t size>
    class IOBlock {
    public:
        using Container = std::array<uint8_t, size>;
        IOBlock(): _curSize(0){}

        IOBlock(IOBlock<size> &&) = default;

        Container &arr() {
            return _arr;
        }

        const Container &carr() const {
            return _arr;
        }

        uint32_t &curSize() {
            return _curSize;
        }

        uint32_t maxSize() {
            return size;
        }
    private:
        Container _arr;
        uint32_t _curSize;
    };

    class IOBlockVect {
    public:
        using Container = ByteVect;
        IOBlockVect(uint_fast32_t size): _curSize(0){
            _arr.resize(size);
        }

        IOBlockVect(IOBlockVect &&) = default;

        IOBlockVect(ByteVect vect) : _curSize(vect.size()), _arr(std::move(vect)){}

        const ByteVect &carr() const {
            return _arr;
        }

        ByteVect &arr() {
            return _arr;
        }

        uint32_t &curSize() {
            return _curSize;
        }

        uint32_t maxSize() {
            return _arr.size();
        }
    private:
        uint32_t _curSize;
        ByteVect _arr;
    };

    template <typename TBlock>
    class IOBlockWrap {
    public:
        IOBlockWrap(TBlock block) :_block(std::move(block)){}

        typename TBlock::Container &arr() {
            return _block.arr();
        }

        const typename TBlock::Container &carr() const {
            return _block.carr();
        }

        uint32_t &curSize() {
            return _block.curSize();
        }

        uint32_t maxSize() {
            return _block.maxSize();
        }

        uint8_t* addr(uint_fast32_t index = 0) {
            return &_block.arr()[0] + index;
        }

        uint8_t* addrCurSize() {
            return &_block.arr()[0] + _block.curSize();
        }

        void reset() {
            _block.curSize() = 0;
        }
    private:
        TBlock _block;
    };

    template <typename T>
    bool operator==(const std::shared_ptr<IOBlockWrap<IOBlockVect>> &ptr, const T &vect) {
        return ptr->carr() == vect;
    }
}
