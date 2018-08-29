#pragma once

#include "common_token.h"

namespace vk_music_fs {
    namespace token{
        class SmallProtobufHelper {
        public:
            SmallProtobufHelper();
            ByteVect getQueryMessage();
            AuthData findVals(const ByteVect &input);
            ByteVect getMTalkRequest(const AuthData &authData);
        private:
            const uint_fast32_t ID_NUM = 7;
            const uint_fast32_t TOKEN_NUM = 8;
            ByteVect writeVarint(uint_fast32_t num);
            uint_fast32_t readVarint(ByteVect::const_iterator &dataBegin, const ByteVect::const_iterator &dataEnd);
            uint64_t read64(ByteVect::const_iterator &dataBegin, const ByteVect::const_iterator &dataEnd);
            void readFieldWtype(
                    ByteVect::const_iterator &dataBegin, const ByteVect::const_iterator &dataEnd,
                    uint_fast32_t &wType, uint_fast32_t &fieldNum
            );

        };
    }
}