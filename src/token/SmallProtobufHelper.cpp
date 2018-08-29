#include "SmallProtobufHelper.h"
#include "ProtobufException.h"
#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>

using namespace vk_music_fs;
using namespace token;

template <typename T1>
ByteVect operator+(const ByteVect& l, const T1& r)
{
    ByteVect c{};
    c.reserve(l.size() + r.size());
    auto bi = std::back_inserter(c);
    std::copy(l.begin(), l.end(), bi);
    std::copy(r.begin(), r.end(), bi);
    return c;
}

SmallProtobufHelper::SmallProtobufHelper() { //NOLINT
}

ByteVect SmallProtobufHelper::getQueryMessage() {
    return {
            0x10, 0x00, 0x1a, 0x2a, 0x31, 0x2d, 0x39, 0x32, 0x39, 0x61, 0x30, 0x64, 0x63, 0x61, 0x30, 0x65, 0x65, 0x65, 0x35, 0x35, 0x35, 0x31, 0x33, 0x32, 0x38, 0x30, 0x31, 0x37, 0x31, 0x61, 0x38, 0x35, 0x38, 0x35, 0x64, 0x61, 0x37, 0x64, 0x63, 0x64, 0x33, 0x37, 0x30, 0x30, 0x66, 0x38, 0x22, 0xe3, 0x01, 0x0a, 0xbf, 0x01, 0x0a, 0x45, 0x67, 0x65, 0x6e, 0x65, 0x72, 0x69, 0x63, 0x5f, 0x78, 0x38, 0x36, 0x2f, 0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x5f, 0x73, 0x64, 0x6b, 0x5f, 0x78, 0x38, 0x36, 0x2f, 0x67, 0x65, 0x6e, 0x65, 0x72, 0x69, 0x63, 0x5f, 0x78, 0x38, 0x36, 0x3a, 0x34, 0x2e, 0x34, 0x2e, 0x32, 0x2f, 0x4b, 0x4b, 0x2f, 0x33, 0x30, 0x37, 0x39, 0x31, 0x38, 0x33, 0x3a, 0x65, 0x6e, 0x67, 0x2f, 0x74, 0x65, 0x73, 0x74, 0x2d, 0x6b, 0x65, 0x79, 0x73, 0x12, 0x06, 0x72, 0x61, 0x6e, 0x63, 0x68, 0x75, 0x1a, 0x0b, 0x67, 0x65, 0x6e, 0x65, 0x72, 0x69, 0x63, 0x5f, 0x78, 0x38, 0x36, 0x2a, 0x07, 0x75, 0x6e, 0x6b, 0x6e, 0x6f, 0x77, 0x6e, 0x32, 0x0e, 0x61, 0x6e, 0x64, 0x72, 0x6f, 0x69, 0x64, 0x2d, 0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x40, 0x85, 0xb5, 0x86, 0x06, 0x4a, 0x0b, 0x67, 0x65, 0x6e, 0x65, 0x72, 0x69, 0x63, 0x5f, 0x78, 0x38, 0x36, 0x50, 0x13, 0x5a, 0x19, 0x41, 0x6e, 0x64, 0x72, 0x6f, 0x69, 0x64, 0x20, 0x53, 0x44, 0x4b, 0x20, 0x62, 0x75, 0x69, 0x6c, 0x74, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x78, 0x38, 0x36, 0x62, 0x07, 0x75, 0x6e, 0x6b, 0x6e, 0x6f, 0x77, 0x6e, 0x6a, 0x0e, 0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x5f, 0x73, 0x64, 0x6b, 0x5f, 0x78, 0x38, 0x36, 0x70, 0x00, 0x10, 0x00, 0x32, 0x06, 0x33, 0x31, 0x30, 0x32, 0x36, 0x30, 0x3a, 0x06, 0x33, 0x31, 0x30, 0x32, 0x36, 0x30, 0x42, 0x0b, 0x6d, 0x6f, 0x62, 0x69, 0x6c, 0x65, 0x3a, 0x4c, 0x54, 0x45, 0x3a, 0x48, 0x00, 0x32, 0x05, 0x65, 0x6e, 0x5f, 0x55, 0x53, 0x38, 0xf0, 0xb4, 0xdf, 0xa6, 0xb9, 0x9a, 0xb8, 0x83, 0x8e, 0x01, 0x52, 0x0f, 0x33, 0x35, 0x38, 0x32, 0x34, 0x30, 0x30, 0x35, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x30, 0x5a, 0x00, 0x62, 0x10, 0x41, 0x6d, 0x65, 0x72, 0x69, 0x63, 0x61, 0x2f, 0x4e, 0x65, 0x77, 0x5f, 0x59, 0x6f, 0x72, 0x6b, 0x70, 0x03, 0x7a, 0x1c, 0x37, 0x31, 0x51, 0x36, 0x52, 0x6e, 0x32, 0x44, 0x44, 0x5a, 0x6c, 0x31, 0x7a, 0x50, 0x44, 0x56, 0x61, 0x61, 0x65, 0x45, 0x48, 0x49, 0x74, 0x64, 0x2b, 0x59, 0x67, 0x3d, 0xa0, 0x01, 0x00, 0xb0, 0x01, 0x00
    };
}

AuthData SmallProtobufHelper::findVals(const ByteVect &input) {
    bool idFound = false;
    bool tokenFound = false;
    auto inpCur = input.cbegin();
    auto inpEnd = input.cend();
    AuthData ret{};
    while(true){
        if(inpCur == inpEnd){
            throw ProtobufException("Empty input");
        }
        uint_fast32_t wtype;
        uint_fast32_t fieldNum;
        this->readFieldWtype(inpCur, inpEnd, wtype, fieldNum);
        switch(wtype){
            case 0:
                this->readVarint(inpCur, inpEnd);
                break;
            case 1:
                if(fieldNum == ID_NUM){
                    idFound = true;
                    ret.id = this->read64(inpCur, inpEnd);
                    std::copy(inpCur - 8, inpCur, std::back_inserter(ret.idBuf));
                } else if(fieldNum == TOKEN_NUM){
                    tokenFound = true;
                    ret.token = this->read64(inpCur, inpEnd);
                } else {
                    inpCur += 8;
                }
                break;
            case 2: {
                auto len = this->readVarint(inpCur, inpEnd);
                inpCur += len;
            }
                break;
            default:
                throw ProtobufException("Unknown type " + std::to_string(wtype));
        }
        if(tokenFound && idFound){
            break;
        }
    }
    return ret;
}

ByteVect SmallProtobufHelper::writeVarint(uint_fast32_t num) {
    ByteVect res;
    while(num != 0){
        auto t = static_cast<uint8_t>(num & 0x7Fu);
        num >>= 7;
        if(num != 0){
            res.push_back(static_cast<uint8_t>(t | 0x80u));
        } else {
            res.push_back(t);
        }
    }
    return res;
}

uint_fast32_t SmallProtobufHelper::readVarint(ByteVect::const_iterator &dataBegin, const ByteVect::const_iterator &dataEnd) {
    uint_fast32_t i = 0;
    uint_fast32_t num = 0;
    auto len = static_cast<uint_fast32_t>(dataEnd - dataBegin);
    while(true){
        if(i == len){
            throw ProtobufException("Bad varint");
        }
        if((*(dataBegin + i) & 0x80u) != 0){
            num = num | ((*(dataBegin + i) ^ 0x80u) << (7 * i));
            i++;
        } else {
            num = num | (*(dataBegin + i) << (7u * i));
            break;
        }
    }
    dataBegin += i + 1;
    return num;
}

uint64_t SmallProtobufHelper::read64(ByteVect::const_iterator &dataBegin, const ByteVect::const_iterator &dataEnd) {
    auto len = static_cast<uint_fast32_t>(dataEnd - dataBegin);
    if(len < 8){
        throw ProtobufException("Bad 64 bit integer");
    }
    uint64_t num = 0;
    for(auto it = dataBegin + 7; it != dataBegin - 1; --it){
        num = (num << 8u) | *it;
    }
    dataBegin = dataBegin + 8;
    return num;
}

ByteVect SmallProtobufHelper::getMTalkRequest(const AuthData &authData) {
    auto strId = std::to_string(authData.id);
    auto strToken = std::to_string(authData.token);
    auto idLen = this->writeVarint(strId.length());
    auto tokenLen = this->writeVarint(strToken.length());
    auto hexIdPrefix = std::string("android-");
    ByteVect hexId;
    boost::algorithm::hex(authData.idBuf.cbegin(), authData.idBuf.cend(), std::back_inserter(hexId));
    auto hexIdLen = this->writeVarint(hexIdPrefix.size() + hexId.size());
    
    auto msg = ByteVect{
            0x0a, 0x0a, 0x61, 0x6e, 0x64, 0x72, 0x6f, 0x69, 0x64, 0x2d, 0x31, 0x39, 0x12, 0x0f, 0x6d, 0x63, 0x73, 0x2e, 0x61, 0x6e, 0x64, 0x72, 0x6f, 0x69, 0x64, 0x2e, 0x63, 0x6f, 0x6d, 0x1a
    } + 
        idLen + strId + ByteVect{0x22} + idLen + strId +
        ByteVect{0x2a} + tokenLen + strToken + ByteVect{0x32} + hexIdLen + hexIdPrefix + hexId
        + ByteVect{0x42, 0x0b, 0x0a, 0x06, 0x6e, 0x65, 0x77, 0x5f, 0x76, 0x63, 0x12, 0x01, 0x31, 0x60, 0x00, 0x70, 0x01, 0x80, 0x01, 0x02, 0x88, 0x01, 0x01}
    ;

    auto len = this->writeVarint(msg.size());

    return ByteVect{0x29, 0x02} + len + msg;
}

void SmallProtobufHelper::readFieldWtype(
        ByteVect::const_iterator &dataBegin, const ByteVect::const_iterator &dataEnd,
        uint_fast32_t &wType, uint_fast32_t &fieldNum
) {
    auto num = readVarint(dataBegin, dataEnd);
    wType = num & 0x7u;
    fieldNum = num >> 3u;
}
