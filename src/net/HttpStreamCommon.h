#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <string>

namespace vk_music_fs {
    namespace net {
        class HttpStreamCommon {
        public:
            const static uint_fast8_t HTTP_VERSION = 11;
            const static uint_fast16_t SSL_PORT = 443;

            HttpStreamCommon();

            typedef boost::asio::ssl::stream<boost::asio::ip::tcp::tcp::socket> Stream;
            struct HostPath {
                std::string host;
                std::string path;
            };

            HostPath getHostPath(const std::string &uri);

            std::shared_ptr<Stream> connect(const HostPath &hostPath);

            void closeStream(const std::shared_ptr<HttpStreamCommon::Stream> &stream);

            void sendGetReq(const std::shared_ptr<HttpStreamCommon::Stream> &stream,
                            const HostPath &hostPath,
                            const std::string &userAgent
            );

            void sendPartialGetReq(const std::shared_ptr<HttpStreamCommon::Stream> &stream,
                                   const HostPath &hostPath,
                                   const std::string &userAgent,
                                   uint_fast32_t byteStart,
                                   uint_fast32_t byteEnd
            );

            void sendHeadReq(const std::shared_ptr<HttpStreamCommon::Stream> &stream,
                             const HostPath &hostPath,
                             const std::string &userAgent);

            std::string uriEncode(const std::string &str);

        private:
            boost::asio::io_context _ioc;
            boost::asio::ssl::context _sslCtx;
            boost::asio::ip::tcp::resolver _resolver;
        };
    }
}
