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
            HttpStream(const RemoteFileUri &uri, const std::shared_ptr<HttpStreamCommon> &common,
                       const UserAgent &userAgent, const HttpTimeout &timeout);

            template <typename T>
            void read(T &block){
                if (_parser.is_done() || _closed) {
                    block->curSize() = 0;
                    return;
                }
                _parser.get().body().data = block->addr();
                _parser.get().body().size = block->maxSize();
                block->curSize() = doRead(block->maxSize());
            }

            void open(uint_fast32_t offset, uint_fast32_t totalSize);

            ByteVect read(uint_fast32_t offset, uint_fast32_t length);

            void close();

        private:
            uint_fast32_t doRead(uint_fast32_t maxSize);

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
        };
    }
}
