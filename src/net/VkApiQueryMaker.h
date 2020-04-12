#pragma once

#include <common/common.h>
#include "HttpStreamCommon.h"
#include "VkSettings.h"

namespace vk_music_fs {
    namespace net {
        class VkApiQueryMaker {
        public:
            VkApiQueryMaker(
                    const std::shared_ptr<HttpStreamCommon> &common,
                    const Token &token,
                    const UserAgent &userAgent,
                    const VkSettings &vkSettings
            );

            std::string makeSearchQuery(const std::string &query, uint_fast32_t offset, uint_fast32_t count);

            std::string makeArtistSearchQuery(const std::string &query, uint_fast32_t offset, uint_fast32_t count);

            std::string makeMyAudiosQuery(uint_fast32_t offset, uint_fast32_t count);

            std::string searchSimilar(const std::string &fileId, uint_fast32_t offset, uint_fast32_t count);

            std::string makeMyPlaylistsQuery(uint_fast32_t ownerId, uint_fast32_t offset, uint_fast32_t count);

            std::string addToMyAudios(int_fast32_t ownerId, uint_fast32_t fileId);

            std::string deleteFromMyAudios(int_fast32_t ownerId, uint_fast32_t fileId);

            std::string getUserId();

            std::string getPlaylistAudios(
                    const std::string &accessKey,
                    int_fast32_t ownerId, uint_fast32_t albumId,
                    uint_fast32_t offset, uint_fast32_t cnt
            );

        private:
            std::shared_ptr<HttpStreamCommon> _common;
            std::string _userAgent;
            std::string _token;
            VkSettings _vkSettings;
        };
    }
}
