#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include "SizeObtainer.h"
#include "HttpException.h"

using namespace vk_music_fs;
using namespace net;

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace ssl = boost::asio::ssl;

Mp3FileSize SizeObtainer::getSize(const std::string &uri, const std::string &artist, const std::string &title) {
    uint_fast32_t size = 0;
    for(uint_fast32_t i = 0; ; i++) {
        try {
            auto hostPath = _common->getHostPath(uri);
            auto stream = _common->connect(hostPath);
            _common->sendHeadReq(stream, hostPath, _userAgent);
            http::response_parser<http::empty_body> parser;
            boost::beast::basic_flat_buffer<std::allocator<uint8_t>> readBuffer;
            parser.skip(true);
            read(*stream, readBuffer, parser);
            size = static_cast<uint_fast32_t>(*parser.content_length());
            break;
        } catch (const boost::system::system_error &ex) {
            if(i == _numSizeRetries){
                throw net::HttpException(std::string("Error obtaining size of uri ") + uri + ". " + ex.what());
            }
        }
    }

    return Mp3FileSize{size, getTagSize(artist, title)};
}

SizeObtainer::SizeObtainer(
        const std::shared_ptr<HttpStreamCommon> &common,
        const UserAgent &userAgent,
        const NumSizeRetries &numSizeRetries
)  : _common(common), _userAgent(userAgent.t), _numSizeRetries(numSizeRetries.t){

}

uint_fast32_t SizeObtainer::getTagSize(const std::string &artist, const std::string &title) {
    TagLib::ID3v2::Tag tag;
    tag.setTitle(title);
    tag.setArtist(artist);
    return tag.render(4).size();
}
