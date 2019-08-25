#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <common/common.h>
#include "HttpStreamCommon.h"

namespace vk_music_fs{
    namespace net {
        class HttpStream {
        public:
            HttpStream(const Mp3Uri &uri, const std::shared_ptr<HttpStreamCommon> &common,
                       const UserAgent &userAgent, const HttpTimeout &timeout);

            void open(uint_fast32_t offset, uint_fast32_t totalSize);

            std::optional<ByteVect> read();

            ByteVect read(uint_fast32_t offset, uint_fast32_t length);

            void close();

        private:
            const uint_fast32_t BUFFER_SIZE = 1024 * 64;

            bool _closed;
            uint_fast32_t _timeout;
            std::string _uri;
            std::string _userAgent;
            std::shared_ptr<HttpStreamCommon> _common;
            HttpStreamCommon::HostPath _hostPath;
            std::shared_ptr<HttpStreamCommon::Stream> _stream;
            boost::beast::basic_flat_buffer<std::allocator<uint8_t>> _readBuffer;
            boost::beast::http::response_parser<boost::beast::http::buffer_body> _parser;
            uint_fast32_t _size;
            ByteVect _returnBuffer;
        };
    }
}
