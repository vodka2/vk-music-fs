#pragma once

#include <gmock/gmock.h>
#include <common/common.h>
#include <mp3core/RemoteFile.h>
#include <common/RemotePhotoFile.h>

using vk_music_fs::ByteVect;
using vk_music_fs::RemoteFile;
using vk_music_fs::RemotePhotoFile;

class PhotoManagerM0{
public:
    template <typename... T>
    PhotoManagerM0(T&&... args){} //NOLINT
    MOCK_CONST_METHOD2(open, int_fast32_t (const RemotePhotoFile &remFile, const std::string &filename));
    MOCK_CONST_METHOD3(read, ByteVect (uint_fast32_t id, uint_fast32_t offset, uint_fast32_t size));
    MOCK_CONST_METHOD1(close, void (uint_fast32_t id));
    MOCK_CONST_METHOD2(getFileSize, uint_fast32_t (const RemotePhotoFile &remFile, const std::string &filename));
};

typedef testing::NiceMock<PhotoManagerM0> PhotoManagerM;
