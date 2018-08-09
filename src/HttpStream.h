#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <common.h>
#include "HttpStreamCommon.h"

namespace vk_music_fs{
    class HttpStream{
    public:
        HttpStream(const std::string &uri, const std::shared_ptr<HttpStreamCommon> &common,
                   const std::string &userAgent);
        uint_fast32_t getSize();
        void open();
        std::optional<ByteVect> read();
        ByteVect read(uint_fast32_t offset, uint_fast32_t length);
    private:
        const uint_fast32_t BUFFER_SIZE = 1024 * 64;

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
