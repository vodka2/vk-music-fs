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
        auto sizeFuture = http::async_read(*_stream, _readBuffer, _parser, boost::asio::use_future);
        if(sizeFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout){
            _stream->next_layer().cancel();
            throw HttpException("Reading aborted on timeout");
        }
        uint_fast32_t size;
        try {
            size = sizeFuture.get();
        } catch (const boost::system::system_error &ex){
            if(ex.code() != boost::beast::http::error::need_buffer){
                throw;
            }
            size = BUFFER_SIZE;
        }

        _returnBuffer.resize(size);
        if (_parser.is_done()) {
            close();
        }
        return _returnBuffer;
    } catch (const boost::system::system_error &ex){
        close();
        throw HttpException(std::string("Error reading uri ") + _uri + ". " + ex.what());
    } catch (const HttpException &ex){
        close();
        throw;
    }
}

ByteVect HttpStream::read(uint_fast32_t offset, uint_fast32_t length) {
    std::shared_ptr<HttpStreamCommon::Stream> stream;
    try {
        if (offset >= _size || _closed) {
            return {};
        }

        stream = _common->connect(_hostPath);
        auto maxReadByte = std::min(offset + length - 1, _size - 1);
        auto readSize = maxReadByte + 1 - offset;
        _common->sendPartialGetReq(stream, _hostPath, _userAgent, offset, maxReadByte);
        ByteVect buf(readSize);
        _common->readPartIntoBuffer(stream, buf);
        return buf;
    } catch (const boost::system::system_error &ex){
        _common->closeStream(stream);
        throw HttpException(std::string("Error reading part of uri ") + _uri + ". " + ex.what());
    } catch (const HttpException &ex){
        close();
        throw;
    }
}

HttpStream::HttpStream(
        const Mp3Uri &uri, const std::shared_ptr<HttpStreamCommon> &common, const UserAgent &userAgent,
        const HttpTimeout &timeout
)
: _timeout(timeout.t), _closed(false),
  _uri(uri.t), _userAgent(userAgent.t), _common(common), _hostPath(_common->getHostPath(uri)){
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
        auto readHeaderFuture = http::async_read_header(*_stream, _readBuffer, _parser, boost::asio::use_future);
        if(readHeaderFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout){
            _stream->next_layer().cancel();
            throw HttpException("Reading headers aborted on timeout");
        }
        readHeaderFuture.get();
        if (
                (offset == 0 && _parser.get().result() != http::status::ok) ||
                (offset != 0 && _parser.get().result() != http::status::partial_content)
        ) {
            throw HttpException(
                    "Bad status code " + std::to_string(static_cast<uint_fast32_t>(_parser.get().result())) +
                    " when opening " + _uri + " when reading");
        }
        if(offset == 0){
            _size = static_cast<uint_fast32_t>(*_parser.content_length());
        }
    } catch (const boost::system::system_error &ex){
        close();
        throw HttpException(std::string("Error opening uri ")  + _uri + ". " + ex.what());
    } catch (const HttpException &ex){
        close();
        throw;
    }
}

void HttpStream::close() {
    if(!_closed) {
        _closed = true;
        _common->closeStream(_stream);
    }
}
