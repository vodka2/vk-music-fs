#pragma once
#include <gmock/gmock.h>
#include <common/common.h>

using vk_music_fs::ByteVect;

class StreamM0{
public:
    template <typename... T>
    StreamM0(T&&... args){} //NOLINT
    MOCK_CONST_METHOD2(read, ByteVect(uint_fast32_t offset, uint_fast32_t size));
    MOCK_CONST_METHOD1(read, void (Block));
    MOCK_CONST_METHOD2(open, void(uint_fast32_t, uint_fast32_t));
    MOCK_CONST_METHOD0(close, void());
};

typedef testing::NiceMock<StreamM0> StreamM;