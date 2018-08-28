#include "HttpStreamCommon.h"
#include "HttpException.h"
#include <regex>
#include <boost/beast/http.hpp>
#include <iomanip>
#include <boost/beast/core.hpp>

using namespace vk_music_fs;
using namespace net;

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace ssl = boost::asio::ssl;

HttpStreamCommon::HostPath HttpStreamCommon::getHostPath(const std::string &uri) {
    std::regex regex("^https://(.+?)(/.+)$");
    std::smatch mtc;
    std::regex_search(uri, mtc, regex);
    HostPath res{mtc[1], mtc[2]};
    return res;
}

std::shared_ptr<HttpStreamCommon::Stream>
HttpStreamCommon::connect(const HostPath &hostPath) {
    auto resolverResultsFuture = _resolver.async_resolve(hostPath.host, std::to_string(SSL_PORT), boost::asio::use_future);
    if(resolverResultsFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout){
        throw HttpException("Resolving aborted on timeout");
    }
    auto resolverResults = resolverResultsFuture.get();
    auto stream = std::make_shared<Stream>(_ioc, _sslCtx);

    if(!SSL_set_tlsext_host_name(stream->native_handle(), hostPath.host.c_str())) {
        boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
        throw boost::system::system_error{ec};
    }

    auto connectFuture = boost::asio::async_connect(stream->next_layer(), resolverResults.begin(), resolverResults.end(), boost::asio::use_future);
    if (connectFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout) {
        stream->next_layer().cancel();
        closeStream(stream);
        throw HttpException("Connection aborted on timeout");
    }
    connectFuture.get();
    stream->handshake(ssl::stream_base::client);
    return stream;
}

void HttpStreamCommon::sendGetReq(
        const std::shared_ptr<HttpStreamCommon::Stream> &stream, const HostPath &hostPath,
        const std::string &userAgent
) {
    http::request<http::string_body> req{http::verb::get, hostPath.path, HttpStreamCommon::HTTP_VERSION};
    req.set(http::field::host, hostPath.host);
    req.set(http::field::user_agent, userAgent);
    auto writeFuture = http::async_write(*stream, req, boost::asio::use_future);
    if(writeFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout){
        stream->next_layer().cancel();
        closeStream(stream);
        throw HttpException("Get request aborted on timeout");
    }
    writeFuture.get();
}

HttpStreamCommon::HttpStreamCommon(
        const std::shared_ptr<ThreadPool> &pool,
        const HttpTimeout &timeout
) :
_resolver{_ioc}, _guard(boost::asio::make_work_guard(_ioc)),
_sslCtx{boost::asio::ssl::context::tlsv12_client}, _timeout(timeout.t){
    _sslCtx.set_options(boost::asio::ssl::context::default_workarounds);
    pool->post([&ioc = _ioc](){
        ioc.run();
    });
}

void HttpStreamCommon::sendHeadReq(const std::shared_ptr<HttpStreamCommon::Stream> &stream,
                                   const HttpStreamCommon::HostPath &hostPath, const std::string &userAgent) {
    http::request<http::string_body> req{http::verb::head, hostPath.path, HttpStreamCommon::HTTP_VERSION};
    req.set(http::field::host, hostPath.host);
    req.set(http::field::user_agent, userAgent);
    auto writeFuture = http::async_write(*stream, req, boost::asio::use_future);
    if(writeFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout){
        stream->next_layer().cancel();
        closeStream(stream);
        throw HttpException("Head request aborted on timeout");
    }
    writeFuture.get();
}

std::string HttpStreamCommon::uriEncode(const std::string &str) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : str) {
        if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char) c);
        escaped << std::nouppercase;
    }

    return escaped.str();
}

void HttpStreamCommon::closeStream(const std::shared_ptr<HttpStreamCommon::Stream> &stream) {
    if(stream != nullptr) {
        boost::beast::error_code ec;
        stream->shutdown(ec);
    }
}

std::string HttpStreamCommon::readRespAsStr(const std::shared_ptr<HttpStreamCommon::Stream> &stream) {
    boost::beast::basic_flat_buffer<std::allocator<uint8_t>> readBuffer;
    http::response_parser<http::string_body> parser;
    auto readHeaderFuture = http::async_read_header(*stream, readBuffer, parser, boost::asio::use_future);
    if(readHeaderFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout){
        stream->next_layer().cancel();
        closeStream(stream);
        throw HttpException("Reading headers aborted on timeout");
    }
    readHeaderFuture.get();
    if (parser.get().result() != http::status::ok) {
        throw HttpException(
                "Bad status code " + std::to_string(static_cast<uint_fast32_t>(parser.get().result()))
        );
    }
    auto readFuture = http::async_read(*stream, readBuffer, parser, boost::asio::use_future);
    if(readFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout){
        stream->next_layer().cancel();
        closeStream(stream);
        throw HttpException("Reading content aborted on timeout");
    }
    try {
        readFuture.get();
    } catch (const boost::system::system_error &ex){
        if(ex.code() != boost::beast::http::error::need_buffer){
            throw;
        }
    }
    auto res = parser.get().body();
    closeStream(stream);
    return res;
}

void HttpStreamCommon::readPartIntoBuffer(
        const std::shared_ptr<HttpStreamCommon::Stream> &stream,
        ByteVect &buf
) {
    boost::beast::basic_flat_buffer<std::allocator<uint8_t>> readBuffer;
    http::response_parser<http::buffer_body> parser;
    auto readHeaderFuture = http::async_read_header(*stream, readBuffer, parser, boost::asio::use_future);
    if(readHeaderFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout){
        stream->next_layer().cancel();
        closeStream(stream);
        throw HttpException("Reading headers aborted on timeout");
    }
    readHeaderFuture.get();
    if (parser.get().result() != http::status::partial_content && parser.get().result() != http::status::ok) {
        throw HttpException(
                "Bad status code " + std::to_string(static_cast<uint_fast32_t>(parser.get().result())) +
                "when reading part"
        );
    }

    parser.get().body().data = &buf[0];
    parser.get().body().size = buf.size();

    auto readFuture = http::async_read(*stream, readBuffer, parser, boost::asio::use_future);
    if(readFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout){
        stream->next_layer().cancel();
        closeStream(stream);
        throw HttpException("Reading content aborted on timeout");
    }
    try {
        readFuture.get();
    } catch (const boost::system::system_error &ex){
        if(ex.code() != boost::beast::http::error::need_buffer){
            throw;
        }
    }
    closeStream(stream);
}

void HttpStreamCommon::sendPartialGetReq(
        const std::shared_ptr<HttpStreamCommon::Stream> &stream,
        const HttpStreamCommon::HostPath &hostPath,
        const std::string &userAgent,
        uint_fast32_t byteStart,
        uint_fast32_t byteEnd
) {
    http::request<http::string_body> req{http::verb::get, hostPath.path, HttpStreamCommon::HTTP_VERSION};
    req.set(http::field::host, hostPath.host);
    req.set(http::field::range, "bytes=" + std::to_string(byteStart) + "-" + std::to_string(byteEnd));
    req.set(http::field::user_agent, userAgent);
    auto writeFuture = http::async_write(*stream, req, boost::asio::use_future);
    if(writeFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout){
        stream->next_layer().cancel();
        closeStream(stream);
        throw HttpException("Partial get request aborted on timeout");
    }
    writeFuture.get();
}

uint_fast32_t HttpStreamCommon::readSize(const std::shared_ptr<HttpStreamCommon::Stream> &stream) {
    http::response_parser<http::empty_body> parser;
    boost::beast::basic_flat_buffer<std::allocator<uint8_t>> readBuffer;
    parser.skip(true);
    auto readFuture = http::async_read(*stream, readBuffer, parser, boost::asio::use_future);
    if(readFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout){
        stream->next_layer().cancel();
        closeStream(stream);
        throw HttpException("Reading content aborted on timeout");
    }
    readFuture.get();
    auto size = static_cast<uint_fast32_t>(*parser.content_length());
    closeStream(stream);
    return size;
}

void HttpStreamCommon::stop() {
    _guard.reset();
}
