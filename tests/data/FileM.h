#pragma once

#include <gmock/gmock.h>
#include <common.h>

using vk_music_fs::ByteVect;

class FileM0{
public:
    template <typename... T>
    FileM0(T&&... args){} //NOLINT
    MOCK_CONST_METHOD2(read, ByteVect(uint_fast32_t offset, uint_fast32_t size));
    MOCK_CONST_METHOD1(write, void(ByteVect vect)); //NOLINT
    MOCK_CONST_METHOD0(finish, void());
    MOCK_CONST_METHOD0(open, void());
    MOCK_CONST_METHOD0(getTotalSize, uint_fast32_t());
    MOCK_CONST_METHOD0(getPrependSize, uint_fast32_t());
    MOCK_CONST_METHOD1(setPrependSize, void(uint_fast32_t));
    MOCK_CONST_METHOD0(close, void());
    MOCK_CONST_METHOD0(getSize, uint_fast32_t());
};

typedef testing::NiceMock<FileM0> FileM;
