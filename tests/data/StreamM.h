#pragma once
#include <gmock/gmock.h>
#include <common.h>

using vk_music_fs::ByteVect;

class StreamM{
public:
    StreamM(){} //NOLINT
    MOCK_CONST_METHOD2(read, ByteVect(uint_fast32_t offset, uint_fast32_t size));
    MOCK_CONST_METHOD0(read, std::optional<ByteVect>());
    MOCK_CONST_METHOD0(getSize, uint_fast32_t());
    MOCK_CONST_METHOD0(open, void());
};
