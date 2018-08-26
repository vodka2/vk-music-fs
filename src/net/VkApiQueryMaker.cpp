#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/core.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "VkApiQueryMaker.h"
#include "HttpException.h"

using namespace vk_music_fs;
using namespace net;

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace ssl = boost::asio::ssl;

VkApiQueryMaker::VkApiQueryMaker(
        const std::shared_ptr<HttpStreamCommon> &common,
        const Token &token,
        const UserAgent &userAgent
) : _common(common), _token(token.t), _userAgent(userAgent.t){

}

std::string VkApiQueryMaker::makeSearchQuery(const std::string &query, uint_fast32_t offset, uint_fast32_t count) {
    std::string uri = "https://api.vk.com/method/audio.search?access_token=" + _token +
    "&q=" + _common->uriEncode(query) + "&offset=" + std::to_string(offset) + "&count=" + std::to_string(count) + "&v=5.71";
    auto hostPath = _common->getHostPath(uri);
    auto stream = _common->connect(hostPath);
    _common->sendGetReq(stream, hostPath, _userAgent);
    boost::beast::basic_flat_buffer<std::allocator<uint8_t>> readBuffer;
    http::response_parser<http::string_body> parser;
    http::read_header(*stream, readBuffer, parser);
    if (parser.get().result() != http::status::partial_content && parser.get().result() != http::status::ok) {
        throw HttpException(
                "Bad status code " + std::to_string(static_cast<uint_fast32_t>(parser.get().result())) +
                " when searching for " + query
        );
    }
    boost::beast::error_code ec;
    http::read(*stream, readBuffer, parser, ec);
    if(ec && ec != boost::beast::http::error::need_buffer){
        throw boost::system::system_error{ec};
    }
    auto res = parser.get().body();
    _common->closeStream(stream);
    return res;
}

std::string VkApiQueryMaker::makeMyAudiosQuery(uint_fast32_t offset, uint_fast32_t count) {
    std::string uri = "https://api.vk.com/method/audio.get?access_token=" + _token +
                      "&offset=" + std::to_string(offset) + "&count=" + std::to_string(count) + "&v=5.71";
    auto hostPath = _common->getHostPath(uri);
    auto stream = _common->connect(hostPath);
    _common->sendGetReq(stream, hostPath, _userAgent);
    boost::beast::basic_flat_buffer<std::allocator<uint8_t>> readBuffer;
    http::response_parser<http::string_body> parser;
    http::read_header(*stream, readBuffer, parser);
    if (parser.get().result() != http::status::partial_content && parser.get().result() != http::status::ok) {
        throw HttpException(
                "Bad status code " + std::to_string(static_cast<uint_fast32_t>(parser.get().result())) +
                " when quering my audios"
        );
    }
    boost::beast::error_code ec;
    http::read(*stream, readBuffer, parser, ec);
    if(ec && ec != boost::beast::http::error::need_buffer){
        throw boost::system::system_error{ec};
    }
    auto res = parser.get().body();
    _common->closeStream(stream);
    return res;
}
