#pragma once

#include <string>
#include <common.h>
#include <mpeg/id3v2/id3v2tag.h>
#include <memory>
#include <HttpStreamCommon.h>

namespace vk_music_fs{
    class SizeObtainer {
    public:
        explicit SizeObtainer(const std::shared_ptr<HttpStreamCommon> &common, const std::string &userAgent);
        Mp3FileSize getSize(const std::string &uri, const std::string &artist, const std::string &title);
        uint_fast32_t getTagSize(const std::string &artist, const std::string &title);
    private:
        std::shared_ptr<HttpStreamCommon> _common;
        std::string _userAgent;
    };
}
