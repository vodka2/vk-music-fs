#pragma once

#include <gmock/gmock.h>
#include <common/common.h>
#include <common/IOBlock.h>

using vk_music_fs::ByteVect;

using Block = std::shared_ptr<vk_music_fs::IOBlockWrap<vk_music_fs::IOBlockVect>>;

template <uint32_t size>
class TestBlockCreator{
public:
    uint_fast32_t vectSize = size;
    template <typename... T>
    TestBlockCreator(T&&... args){} //NOLINT
    using BlockPtr = Block;
    BlockPtr create(){
        return std::make_shared<vk_music_fs::IOBlockWrap<vk_music_fs::IOBlockVect>>(vk_music_fs::IOBlockVect{vectSize});
    }

    void setSize(uint_fast32_t _size) {
        vectSize = _size;
    }
};

