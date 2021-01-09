#include <utility>
#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <json.hpp>
#include <random>
#include <net/HttpException.h>

#include "TokenException.h"
#include "TokenReceiver.h"

using namespace vk_music_fs;
using namespace token;

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace ssl = boost::asio::ssl;
using json = nlohmann::json;

TokenReceiver::TokenReceiver(
        VkCredentials creds, const std::shared_ptr<net::HttpStreamCommon> &common,
        const UserAgent &userAgent, const HttpTimeout &timeout, const net::VkSettings &vkSettings
        )
    :_timeout(timeout.t), _creds(std::move(creds)), _common(common), _userAgent(userAgent.t), _vkSettings(vkSettings){
}

std::string TokenReceiver::getToken() {
    doCheckin();
    sendMTalkReq();
    getNonRefreshedToken();
    gmsRegister(1, "GCM");
    auto resStr = gmsRegister(2, "id" + std::to_string(_vkId));
    _receipt = resStr.substr(resStr.find(':') + 1);
    if(_receipt == "PHONE_REGISTRATION_ERROR"){
        throw TokenException("Registration error");
    }
    refreshToken();
    return _token;
}

std::string TokenReceiver::getUserAgent() {
    return "KateMobileAndroid/40.4 lite-394 (Android 4.4.2; SDK 19; x86; unknown Android SDK built for x86; en)";
}

void TokenReceiver::doCheckin() {
    auto hostPath = _common->getHostPath("https://android.clients.google.com/checkin");
    auto stream = _common->connect(hostPath);
    _common->sendPostReq(stream, hostPath, _userAgent, _protoHelper.getQueryMessage(), "application/x-protobuffer");
    ByteVect res = _common->readIntoBuffer(stream);
    _common->closeStream(stream);
    _authData = _protoHelper.findVals(res);
}

void TokenReceiver::sendMTalkReq() {
    auto stream = _common->connect({"mtalk.google.com", ""}, 5228);
    auto writeFuture = boost::asio::async_write(
            *stream, boost::asio::buffer(_protoHelper.getMTalkRequest(_authData)), boost::asio::use_future
    );
    if(writeFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout){
        stream->next_layer().cancel();
        _common->closeStream(stream);
        throw TokenException("MTalk write request aborted on timeout");
    }
    writeFuture.get();
    std::array<uint8_t, 2> data{};
    auto readFuture = boost::asio::async_read(*stream, boost::asio::buffer(&data[0], data.size()), boost::asio::use_future);
    if(readFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout){
        stream->next_layer().cancel();
        _common->closeStream(stream);
        throw TokenException("Mtalk read response aborted on timeout");
    }
    readFuture.get();
    _common->closeStream(stream);
    const uint8_t SUCCESS_CODE = 3;
    if(data[1] != SUCCESS_CODE){
        throw TokenException("Mtalk bad response");
    }
}

void TokenReceiver::getNonRefreshedToken() {
    auto hostPath = _common->getHostPath(
            "https://oauth.vk.com/token?grant_type=password&client_id=2685278&client_secret=lxhD8OD7dMsqtXIm5IUY&username=" +
            _common->uriEncode(_creds.login) + "&password=" +
            _common->uriEncode(_creds.password) + "&v=" + _vkSettings.version + "&scope=" + _common->uriEncode("audio,offline")
    );
    auto stream = _common->connect(hostPath);
    _common->sendGetReq(stream, hostPath, _userAgent);
    auto respStr = _common->readRespAsStr(stream, false);
    _common->closeStream(stream);
    try {
        auto respJson = json::parse(respStr);
        if(respJson.count("user_id") == 0 || respJson.count("access_token") == 0){
            throw TokenException("No user id or access token in response. Response: " + respStr);
        }
        _vkId = respJson["user_id"];
        _nonRefreshedToken = respJson["access_token"];
    } catch (const json::parse_error &){
        throw TokenException("Error parsing JSON when receiving token. Response: " + respStr);
    }
}

void TokenReceiver::refreshToken() {
    auto hostPath = _common->getHostPath(
            _vkSettings.apiUriPrefix + "/auth.refreshToken?access_token=" + _nonRefreshedToken +
            "&receipt=" + _receipt + "&v=" + _vkSettings.version
    );
    auto stream = _common->connect(hostPath);
    _common->sendGetReq(stream, hostPath, _userAgent);
    auto respStr = _common->readRespAsStr(stream, false);
    try {
        auto respJson = json::parse(respStr);
        _token = respJson["response"]["token"];
    } catch (const json::parse_error &){
        throw TokenException("Error parsing JSON when refreshing token. Response: " + respStr);
    }
    if(_token == _nonRefreshedToken){
        throw TokenException("Token was not refreshed");
    }
    _common->closeStream(stream);
}

std::string TokenReceiver::generateRandomString(uint_fast16_t length) {
    static std::string chrs = "0123456789"
                        "abcdefghijklmnopqrstuvwxyz"
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ_-";

    thread_local std::mt19937 rg{std::random_device{}()};
    thread_local std::uniform_int_distribution<std::string::size_type> pick(0, chrs.length() - 1);

    std::string s;
    s.reserve(length);
    while(length--) {
        s += chrs[pick(rg)];
    }
    return s;
}

std::string TokenReceiver::gmsRegister(uint_fast32_t id, const std::string &scope) {
    auto hostPath = _common->getHostPath("https://android.clients.google.com/c2dm/register3");
    auto stream = _common->connect(hostPath);
    http::request<http::string_body> req{http::verb::post, hostPath.path, net::HttpStreamCommon::HTTP_VERSION};
    req.set(http::field::host, hostPath.host);
    req.set(http::field::user_agent, _userAgent);
    req.set(http::field::content_type, "application/x-www-form-urlencoded");
    req.set("Authorization", "AidLogin " + std::to_string(_authData.id) + ":" + std::to_string(_authData.token));
    auto data = _common->fieldsToPostReq({
                                                 {"X-scope", scope},
                                                 {"X-osv", "23"},
                                                 {"X-subtype", "54740537194"},
                                                 {"X-app_ver", "434"},
                                                 {"X-kid", "|ID|" + std::to_string(id) + "|"},
                                                 {"X-appid", generateRandomString(11)},
                                                 {"X-gmsv", "9452480"},
                                                 {"X-cliv", "iid-9452000"},
                                                 {"X-app_ver_name", "49 lite"},
                                                 {"X-X-kid", "|ID|" + std::to_string(id) + "|"},
                                                 {"X-subscription", "54740537194"},
                                                 {"X-X-subscription", "54740537194"},
                                                 {"X-X-subtype", "54740537194"},
                                                 {"app", "com.perm.kate_new_6"},
                                                 {"sender", "54740537194"},
                                                 {"device", std::to_string(_authData.id)},
                                                 {"cert", "966882ba564c2619d55d0a9afd4327a38c327456"},
                                                 {"app_ver", "434"},
                                                 {"info", "IwSu2g51cjoRUO6eTSP7b7VIl0qkOxY"},
                                                 {"gcm_ver", "9452480"}
                                         });
    req.set(http::field::content_length, std::to_string(data.size()));
    req.body() = data;
    auto writeFuture = http::async_write(*stream, req, boost::asio::use_future);
    if(writeFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout){
        stream->next_layer().cancel();
        _common->closeStream(stream);
        throw net::HttpException("Post request aborted on timeout");
    }
    writeFuture.get();
    auto resStr = _common->readRespAsStr(stream, false);
    _common->closeStream(stream);
    return resStr;
}
