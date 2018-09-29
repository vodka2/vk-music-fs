#include <boost/beast/http.hpp>
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
    std::shared_ptr<HttpStreamCommon::Stream> stream;
    try {
        std::string uri = "https://api.vk.com/method/audio.search?access_token=" + _token +
                          "&q=" + _common->uriEncode(query) + "&offset=" + std::to_string(offset) + "&count=" +
                          std::to_string(count) + "&v=5.71";
        auto hostPath = _common->getHostPath(uri);
        stream = _common->connect(hostPath);
        _common->sendGetReq(stream, hostPath, _userAgent);
        return _common->readRespAsStr(stream);
    } catch (const boost::system::system_error &ex){
        _common->closeStream(stream);
        throw HttpException(std::string("Error searching for ") + query + ". " + ex.what());
    }
}

std::string VkApiQueryMaker::makeMyAudiosQuery(uint_fast32_t offset, uint_fast32_t count) {
    std::shared_ptr<HttpStreamCommon::Stream> stream;
    try{
        std::string uri = "https://api.vk.com/method/audio.get?access_token=" + _token +
                          "&offset=" + std::to_string(offset) + "&count=" + std::to_string(count) + "&v=5.71";
        auto hostPath = _common->getHostPath(uri);
        stream = _common->connect(hostPath);
        _common->sendGetReq(stream, hostPath, _userAgent);
        return _common->readRespAsStr(stream);
    } catch (const boost::system::system_error &ex){
        _common->closeStream(stream);
        throw HttpException(std::string("Error querying my audios. ") + ex.what());
    }
}

std::string
VkApiQueryMaker::makeArtistSearchQuery(const std::string &query, uint_fast32_t offset, uint_fast32_t count) {
    std::shared_ptr<HttpStreamCommon::Stream> stream;
    try {
        std::string uri = "https://api.vk.com/method/audio.search?access_token=" + _token + "&performer_only=1"
                          "&q=" + _common->uriEncode(query) + "&offset=" + std::to_string(offset) + "&count=" +
                          std::to_string(count) + "&v=5.71";
        auto hostPath = _common->getHostPath(uri);
        stream = _common->connect(hostPath);
        _common->sendGetReq(stream, hostPath, _userAgent);
        return _common->readRespAsStr(stream);
    } catch (const boost::system::system_error &ex){
        _common->closeStream(stream);
        throw HttpException(std::string("Error searching in artists for ") + query + ". " + ex.what());
    }
}
