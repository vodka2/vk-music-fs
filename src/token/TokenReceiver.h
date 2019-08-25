#pragma once

#include "common_token.h"
#include "SmallProtobufHelper.h"
#include <net/HttpStreamCommon.h>
#include <net/VkSettings.h>

namespace vk_music_fs {
    namespace token {
        class TokenReceiver {
        public:
            TokenReceiver(
                    VkCredentials creds, const std::shared_ptr<net::HttpStreamCommon> &common,
                    const UserAgent &userAgent, const HttpTimeout &timeout, const net::VkSettings &vkSettings
            );

            std::string getToken();
            static std::string getUserAgent();
        private:
            void doCheckin();
            void sendMTalkReq();
            void getNonRefreshedToken();
            void refreshToken();
            std::string gmsRegister(uint_fast32_t id, const std::string &scope);
            std::string generateRandomString(uint_fast16_t length);
            AuthData _authData;
            uint_fast32_t _vkId;
            std::string _nonRefreshedToken;
            std::string _token;
            std::string _receipt;
            uint_fast32_t _timeout;
            std::shared_ptr<net::HttpStreamCommon> _common;
            std::string _userAgent;
            VkCredentials _creds;
            SmallProtobufHelper _protoHelper;
            net::VkSettings _vkSettings;
        };
    }
}
