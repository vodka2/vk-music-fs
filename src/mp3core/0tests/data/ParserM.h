#pragma once

#include <BlockingBuffer.h>
#include <gmock/gmock.h>
#include <common/common.h>

template <typename TBuffer>
class ParserM0{
public:
    template <typename... T>
    ParserM0(T&&... args){} //NOLINT
    MOCK_CONST_METHOD1_T(parse, void(const std::shared_ptr<TBuffer> &vect));
};

template <typename T>
using ParserM = testing::NiceMock<ParserM0<T>>;