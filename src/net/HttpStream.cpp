#include "HttpStream.h"
#include "HttpException.h"

using namespace vk_music_fs;
using namespace net;

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace ssl = boost::asio::ssl;

std::optional<ByteVect> HttpStream::read() {
    try {
        if (_parser.is_done() || _closed) {
            return std::nullopt;
        }
        _returnBuffer.resize(BUFFER_SIZE);
        _parser.get().body().data = &_returnBuffer[0];
        _parser.get().body().size = BUFFER_SIZE;
        boost::beast::error_code ec;
        auto size = http::read(*_stream, _readBuffer, _parser, ec);
        if(ec && ec != boost::beast::http::error::need_buffer){
            throw boost::system::system_error{ec};
        }
        _returnBuffer.resize(size);
        if (_parser.is_done()) {
            _common->closeStream(_stream);
        }
        return _returnBuffer;
    } catch (const boost::system::system_error &ex){
        throw HttpException(std::string("Error reading uri ") + _uri + ". " + ex.what());
    }
}

ByteVect HttpStream::read(uint_fast32_t offset, uint_fast32_t length) {
    try {
        if (offset >= _size || _closed) {
            return {};
        }

        auto stream = _common->connect(_hostPath);
        auto maxReadByte = std::min(offset + length - 1, _size - 1);
        auto readSize = maxReadByte + 1 - offset;
        _common->sendPartialGetReq(stream, _hostPath, _userAgent, offset, maxReadByte);

        boost::beast::basic_flat_buffer<std::allocator<uint8_t>> readBuffer;
        http::response_parser<http::buffer_body> parser;
        http::read_header(*stream, readBuffer, parser);
        if (parser.get().result() != http::status::partial_content && parser.get().result() != http::status::ok) {
            throw HttpException(
                "Bad status code " + std::to_string(static_cast<uint_fast32_t>(parser.get().result())) +
                " when opening " + _uri + " when reading part"
            );
        }
        ByteVect buf(readSize);

        parser.get().body().data = &buf[0];
        parser.get().body().size = readSize;

        boost::beast::error_code ec;
        http::read(*stream, readBuffer, parser, ec);
        if(ec && ec != boost::beast::http::error::need_buffer){
            throw boost::system::system_error{ec};
        }
        _common->closeStream(stream);
        return buf;
    } catch (const boost::system::system_error &ex){
        throw HttpException(std::string("Error reading part of uri ") + _uri + ". " + ex.what());
    }
}

HttpStream::HttpStream(const Mp3Uri &uri, const std::shared_ptr<HttpStreamCommon> &common, const UserAgent &userAgent)
: _closed(false), _uri(uri.t), _userAgent(userAgent.t), _common(common), _hostPath(_common->getHostPath(uri)){
    _parser.body_limit(std::numeric_limits<std::uint_fast32_t>::max());
}

void HttpStream::open(uint_fast32_t offset, uint_fast32_t totalSize) {
    try {
        _stream = _common->connect(_hostPath);
        if(offset == 0) {
            _common->sendGetReq(_stream, _hostPath, _userAgent);
        } else {
            _size = totalSize;
            _common->sendPartialGetReq(_stream, _hostPath, _userAgent, offset, _size - 1);
        }
        http::read_header(*_stream, _readBuffer, _parser);
        if (
                (offset == 0 && _parser.get().result() != http::status::ok) ||
                (offset != 0 && _parser.get().result() != http::status::partial_content)
        ) {
            throw HttpException(
                    "Bad status code " + std::to_string(static_cast<uint_fast32_t>(_parser.get().result())) +
                    " when opening " + _uri + " when reading");
        }
        if(offset == 0){
            _size = *_parser.content_length();
        }
    } catch (const boost::system::system_error &ex){
        throw HttpException(std::string("Error opening uri ")  + _uri + ". " + ex.what());
    }
}

void HttpStream::close() {
    if(!_closed) {
        try {
            _closed = true;
            _common->closeStream(_stream);
        } catch (const boost::system::system_error &ex) {
            throw HttpException(std::string("Error closing stream ") + _uri);
        }
    }
}
