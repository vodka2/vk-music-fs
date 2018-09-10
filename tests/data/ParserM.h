#pragma once

#include <BlockingBuffer.h>
#include <gmock/gmock.h>
#include <common.h>

class ParserM0{
public:
    template <typename... T>
    ParserM0(T&&... args){} //NOLINT
    MOCK_CONST_METHOD1(parse, void(const std::shared_ptr<vk_music_fs::BlockingBuffer> &vect));
};

typedef testing::NiceMock<ParserM0> ParserM;