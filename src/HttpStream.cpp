#include "HttpStream.h"
#include "HttpException.h"

using namespace vk_music_fs;

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace ssl = boost::asio::ssl;

std::optional<ByteVect> HttpStream::read() {
    try {
        if (_parser.is_done()) {
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
            close();
        }
        return _returnBuffer;
    } catch (const boost::system::system_error &ex){
        throw HttpException(std::string("Error reading uri ") + _uri + ". " + ex.what());
    }
}

ByteVect HttpStream::read(uint_fast32_t offset, uint_fast32_t length) {
    try {
        if (offset >= _size) {
            return {};
        }

        auto stream = _common->connect(_hostPath);
        http::request<http::string_body> req{http::verb::get, _hostPath.path, HttpStreamCommon::HTTP_VERSION};
        req.set(http::field::host, _hostPath.host);
        auto maxReadByte = std::min(offset + length - 1, _size - 1);
        auto readSize = maxReadByte + 1 - offset;
        req.set(http::field::range, "bytes=" + std::to_string(offset) + "-" + std::to_string(maxReadByte));
        req.set(http::field::user_agent, _userAgent);
        http::write(*stream, req);

        boost::beast::basic_flat_buffer<std::allocator<uint8_t>> readBuffer;
        http::response_parser<http::buffer_body> parser;
        http::read_header(*stream, readBuffer, parser);
        if (parser.get().result() != http::status::partial_content && _parser.get().result() != http::status::ok) {
            throw HttpException(
                "Bad status code " + std::to_string(static_cast<uint_fast32_t>(parser.get().result())) +
                " when opening " + _uri
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
        return buf;
    } catch (const boost::system::system_error &ex){
        throw HttpException(std::string("Error reading part of uri ") + _uri + ". " + ex.what());
    }
}

HttpStream::HttpStream(const Mp3Uri &uri, const std::shared_ptr<HttpStreamCommon> &common, const UserAgent &userAgent)
: _uri(uri.t), _userAgent(userAgent.t), _common(common), _hostPath(_common->getHostPath(uri)){
    _parser.body_limit(std::numeric_limits<std::uint_fast32_t>::max());
}

void HttpStream::open(uint_fast32_t offset, uint_fast32_t totalSize) {
    try {
        _size = totalSize;
        _stream = _common->connect(_hostPath);
        if(offset == 0) {
            _common->sendGetReq(_stream, _hostPath, _userAgent);
        }
        http::read_header(*_stream, _readBuffer, _parser);
        if (_parser.get().result() != http::status::ok) {
            throw HttpException(
                    "Bad status code " + std::to_string(static_cast<uint_fast32_t>(_parser.get().result())) +
                    " when opening " + _uri);
        }
    } catch (const boost::system::system_error &ex){
        throw HttpException(std::string("Error opening uri ")  + _uri + ". " + ex.what());
    }
}

void HttpStream::close() {
    boost::beast::error_code ec;
    _stream->shutdown(ec);
    if (ec && ec != boost::asio::ssl::error::stream_truncated) {
        throw HttpException(std::string("Error closing stream ") + _uri);
    }
}
