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
        const UserAgent &userAgent,
        const VkSettings &vkSettings
) : _common(common), _token(token.t), _userAgent(userAgent.t), _vkSettings(vkSettings){

}

std::string VkApiQueryMaker::makeSearchQuery(const std::string &query, uint_fast32_t offset, uint_fast32_t count) {
    std::shared_ptr<HttpStreamCommon::Stream> stream;
    try {
        std::string uri = _vkSettings.apiUriPrefix + "/audio.search?access_token=" + _token +
                          "&q=" + _common->uriEncode(query) + "&offset=" + std::to_string(offset) + "&count=" +
                          std::to_string(count) + "&v=" + _vkSettings.version;
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
        std::string uri = _vkSettings.apiUriPrefix + "/audio.get?access_token=" + _token +
                          "&offset=" + std::to_string(offset) + "&count=" + std::to_string(count) +
                          "&v=" + _vkSettings.version;
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
        std::string uri = _vkSettings.apiUriPrefix + "/audio.search?access_token=" + _token + "&performer_only=1"
                          "&q=" + _common->uriEncode(query) + "&offset=" + std::to_string(offset) + "&count=" +
                          std::to_string(count) + "&v=" + _vkSettings.version;
        auto hostPath = _common->getHostPath(uri);
        stream = _common->connect(hostPath);
        _common->sendGetReq(stream, hostPath, _userAgent);
        return _common->readRespAsStr(stream);
    } catch (const boost::system::system_error &ex){
        _common->closeStream(stream);
        throw HttpException(std::string("Error searching in artists for ") + query + ". " + ex.what());
    }
}

std::string VkApiQueryMaker::addToMyAudios(int_fast32_t ownerId, uint_fast32_t fileId) {
    std::shared_ptr<HttpStreamCommon::Stream> stream;
    try {
        std::string uri = _vkSettings.apiUriPrefix + "/audio.add?access_token=" +
                _token + "&audio_id=" + std::to_string(fileId) + "&owner_id=" + std::to_string(ownerId) +
                "&v=" + _vkSettings.version;
        auto hostPath = _common->getHostPath(uri);
        stream = _common->connect(hostPath);
        _common->sendGetReq(stream, hostPath, _userAgent);
        return _common->readRespAsStr(stream);
    } catch (const boost::system::system_error &ex){
        _common->closeStream(stream);
        throw HttpException(
                std::string("Error adding file ") +
                std::to_string(ownerId) + ":" + std::to_string(fileId) +
                ". " + ex.what()
        );
    }
}

std::string VkApiQueryMaker::deleteFromMyAudios(int_fast32_t ownerId, uint_fast32_t fileId) {
    std::shared_ptr<HttpStreamCommon::Stream> stream;
    try {
        std::string uri = _vkSettings.apiUriPrefix + "/audio.delete?access_token=" +
                          _token + "&audio_id=" + std::to_string(fileId) + "&owner_id=" + std::to_string(ownerId) +
                          "&v=" + _vkSettings.version;
        auto hostPath = _common->getHostPath(uri);
        stream = _common->connect(hostPath);
        _common->sendGetReq(stream, hostPath, _userAgent);
        return _common->readRespAsStr(stream);
    } catch (const boost::system::system_error &ex){
        _common->closeStream(stream);
        throw HttpException(
                std::string("Error deleting file ") +
                ". " + ex.what()
        );
    }
}

std::string VkApiQueryMaker::makeMyPlaylistsQuery(uint_fast32_t ownerId, uint_fast32_t offset, uint_fast32_t count) {
    std::shared_ptr<HttpStreamCommon::Stream> stream;
    try {
        std::string uri = _vkSettings.apiUriPrefix + "/audio.getPlaylists?access_token=" +
                          _token + "&count=" + std::to_string(count) + "&offset=" + std::to_string(offset) +
                          "&owner_id=" + std::to_string(ownerId) + "&v=" + _vkSettings.version;
        auto hostPath = _common->getHostPath(uri);
        stream = _common->connect(hostPath);
        _common->sendGetReq(stream, hostPath, _userAgent);
        return _common->readRespAsStr(stream);
    } catch (const boost::system::system_error &ex){
        _common->closeStream(stream);
        throw HttpException(
                std::string("Error getting playlists ") + ex.what()
        );
    }
}

std::string VkApiQueryMaker::getUserId() {
    std::shared_ptr<HttpStreamCommon::Stream> stream;
    try {
        std::string uri = _vkSettings.apiUriPrefix + "/users.get?access_token=" + _token + "&v=" + _vkSettings.version;
        auto hostPath = _common->getHostPath(uri);
        stream = _common->connect(hostPath);
        _common->sendGetReq(stream, hostPath, _userAgent);
        return _common->readRespAsStr(stream);
    } catch (const boost::system::system_error &ex){
        _common->closeStream(stream);
        throw HttpException(
                std::string("Error getting user id ") + ex.what()
        );
    }
}

std::string
VkApiQueryMaker::getPlaylistAudios(const std::string &accessKey, int_fast32_t ownerId, uint_fast32_t albumId,
                                   uint_fast32_t offset, uint_fast32_t cnt) {
    std::shared_ptr<HttpStreamCommon::Stream> stream;
    try {
        std::string uri = _vkSettings.apiUriPrefix + "/audio.get?access_token=" + _token + "&access_key=" +
                accessKey + "&album_id=" + std::to_string(albumId) + "&offset=" + std::to_string(offset) +
                "&count=" + std::to_string(cnt) + "&owner_id=" + std::to_string(ownerId) + "&v=" + _vkSettings.version;
        auto hostPath = _common->getHostPath(uri);
        stream = _common->connect(hostPath);
        _common->sendGetReq(stream, hostPath, _userAgent);
        return _common->readRespAsStr(stream);
    } catch (const boost::system::system_error &ex){
        _common->closeStream(stream);
        throw HttpException(
                std::string("Error getting playlist ") +
                " album_id:" + std::to_string(albumId) + " owner_id:" + std::to_string(ownerId) + "." +
                ex.what()
        );
    }
}
