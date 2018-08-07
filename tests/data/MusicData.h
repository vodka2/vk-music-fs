#pragma once

#include <common.h>
#include <optional>
#include <thread>

using vk_music_fs::ByteVect;

class MusicData{
private:
    ByteVect _data;
    uint_fast32_t _offset;
    const uint_fast32_t _len;
public:
    explicit MusicData(ByteVect data, uint_fast32_t len) : _data(std::move(data)), _offset(0), _len(len){}
    std::optional<ByteVect> readData(){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ByteVect res;
        if(_offset >= _data.size()){
            return std::nullopt;
        }
        uint_fast32_t realLen = std::min(_offset + _len, _data.size());
        std::copy(_data.cbegin() + _offset, _data.cbegin() + realLen, std::back_inserter(res));
        _offset += _len;
        return res;
    }
};
