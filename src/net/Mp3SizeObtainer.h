#pragma once

#include <string>
#include <common.h>
#include <mpeg/id3v2/id3v2tag.h>
#include <memory>
#include "HttpStreamCommon.h"

namespace vk_music_fs{
    namespace net {
        class Mp3SizeObtainer {
        public:
            explicit Mp3SizeObtainer(
                    const std::shared_ptr<HttpStreamCommon> &common, const UserAgent &userAgent,
                    const NumSizeRetries &numSizeRetries
            );

            uint_fast32_t getSize(const std::string &uri);

        private:
            std::shared_ptr<HttpStreamCommon> _common;
            uint_fast32_t _numSizeRetries;
            std::string _userAgent;
        };
    }
}
