#include "HttpStreamCommon.h"
#include <regex>
#include <boost/asio/connect.hpp>
#include <boost/beast/http.hpp>

using namespace vk_music_fs;

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
    auto resolverResults = _resolver.resolve(hostPath.host, std::to_string(SSL_PORT));
    auto stream = std::make_shared<Stream>(_ioc, _sslCtx);

    if(!SSL_set_tlsext_host_name(stream->native_handle(), hostPath.host.c_str())) {
        boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
        throw boost::system::system_error{ec};
    }

    boost::asio::connect(stream->next_layer(), resolverResults.begin(), resolverResults.end());
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
    http::write(*stream, req);
}

HttpStreamCommon::HttpStreamCommon() : _resolver{_ioc}, _sslCtx{boost::asio::ssl::context::tlsv12_client}{
    _sslCtx.set_options(boost::asio::ssl::context::default_workarounds);
}

void HttpStreamCommon::sendHeadReq(const std::shared_ptr<HttpStreamCommon::Stream> &stream,
                                   const HttpStreamCommon::HostPath &hostPath, const std::string &userAgent) {
    http::request<http::string_body> req{http::verb::head, hostPath.path, HttpStreamCommon::HTTP_VERSION};
    req.set(http::field::host, hostPath.host);
    req.set(http::field::user_agent, userAgent);
    http::write(*stream, req);
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
    http::write(*stream, req);
}
