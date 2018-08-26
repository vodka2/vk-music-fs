#pragma once

#include <common.h>
#include "HttpStreamCommon.h"

namespace vk_music_fs {
    namespace net {
        class VkApiQueryMaker {
        public:
            VkApiQueryMaker(
                    const std::shared_ptr<HttpStreamCommon> &common,
                    const Token &token,
                    const UserAgent &userAgent
            );

            std::string makeSearchQuery(const std::string &query, uint_fast32_t offset, uint_fast32_t count);

            std::string makeMyAudiosQuery(uint_fast32_t offset, uint_fast32_t count);

        private:
            std::shared_ptr<HttpStreamCommon> _common;
            std::string _userAgent;
            std::string _token;
        };
    }
}
