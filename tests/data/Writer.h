#pragma once

#include <common.h>

using vk_music_fs::ByteVect;

class Writer{
public:
    ByteVect data;
    void write(const ByteVect &_data){
        std::copy(_data.cbegin(), _data.cend(), std::back_inserter(data));
    }
    uint_fast32_t getSize(){
        return data.size();
    }
};
