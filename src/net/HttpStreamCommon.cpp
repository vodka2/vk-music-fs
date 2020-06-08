#include "HttpStreamCommon.h"
#include "WrongSizeException.h"
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
HttpStreamCommon::connect(const HostPath &hostPath, uint_fast32_t port) {
    std::promise<std::shared_ptr<Stream>> prom;
    connect(hostPath, [&prom] (auto strm) {
        prom.set_value(strm);
    }, [&prom] (auto ptr) {
        prom.set_exception(ptr);
    }, port);
    return prom.get_future().get();
}

void HttpStreamCommon::connect(
        const HttpStreamCommon::HostPath &hostPath,
        std::function<void(std::shared_ptr<Stream>)> succHandler,
        std::function<void(std::exception_ptr)> errHandler,
        uint_fast32_t port
) {
    auto afterConnectFunc = [succHandler] (auto stream) {
        stream->handshake(ssl::stream_base::client);
        succHandler(stream);
    };
    auto connectFunc = [this, hostPath, afterConnectFunc, errHandler] (auto resolverResults) {
        auto stream = std::make_shared<Stream>(_ioc, _sslCtx);
        if (!SSL_set_tlsext_host_name(stream->native_handle(), hostPath.host.c_str())) {
            boost::system::error_code ec{static_cast<int>(::ERR_get_error()),
                                         boost::asio::error::get_ssl_category()};
            throw boost::system::system_error{ec};
        }
        boost::asio::async_connect(stream->next_layer(), resolverResults.begin(), resolverResults.end(), createTimer(
                [afterConnectFunc, stream](auto &&...) {
                    afterConnectFunc(stream);
                },
                [this, stream, errHandler](auto exc) {
                    stream->next_layer().cancel();
                    closeStream(stream);
                    errHandler(exc);
                }, "Connection aborted on timeout"
        ));
    };
    auto resolveFunc = [this, hostPath, port, succHandler, errHandler, connectFunc] {
        _resolver.async_resolve(hostPath.host, std::to_string(port), createTimer(
                connectFunc, errHandler, "Resolving aborted on timeout"
        ));
    };
    boost::asio::post(_ioc, resolveFunc);
}

void HttpStreamCommon::sendGetReq(
        const std::shared_ptr<HttpStreamCommon::Stream> &stream, const HostPath &hostPath,
        const std::string &userAgent
) {
    std::promise<void> prom;
    sendGetReq(stream, hostPath, userAgent, [&prom] () {
        prom.set_value();
    }, [&prom] (auto ptr) {
        prom.set_exception(ptr);
    });
    return prom.get_future().get();
}

void HttpStreamCommon::sendGetReq(const std::shared_ptr<HttpStreamCommon::Stream> &stream,
                                  const HttpStreamCommon::HostPath &hostPath, const std::string &userAgent,
                                  std::function<void()> succHandler,
                                  std::function<void(std::exception_ptr)> errHandler) {
    auto writeFunc = [this, stream, succHandler, errHandler, hostPath, userAgent] {
        auto req = std::make_shared<http::request<http::string_body>>(http::verb::get, hostPath.path, HttpStreamCommon::HTTP_VERSION);
        req->set(http::field::host, hostPath.host);
        req->set(http::field::user_agent, userAgent);
        http::async_write(*stream, *req, createTimer(
                [succHandler, req](...) { succHandler(); }, [this, errHandler, stream](auto exc) {
                    stream->next_layer().cancel();
                    closeStream(stream);
                    errHandler(exc);
                }, "Get request aborted on timeout"
        ));
    };
    boost::asio::post(_ioc, writeFunc);
}

HttpStreamCommon::HttpStreamCommon(
        const std::shared_ptr<ThreadPool> &pool,
        const HttpTimeout &timeout
) :
_resolver{_ioc}, _guard(boost::asio::make_work_guard(_ioc)),
_sslCtx{boost::asio::ssl::context::tlsv12_client}, _timeout(timeout.t) {
    _sslCtx.set_options(boost::asio::ssl::context::default_workarounds);
    pool->post([&ioc = _ioc]() {
        ioc.run();
    });
}

void HttpStreamCommon::sendHeadReq(const std::shared_ptr<HttpStreamCommon::Stream> &stream,
                                   const HttpStreamCommon::HostPath &hostPath, const std::string &userAgent) {
    http::request<http::string_body> req{http::verb::head, hostPath.path, HttpStreamCommon::HTTP_VERSION};
    req.set(http::field::host, hostPath.host);
    req.set(http::field::user_agent, userAgent);
    auto writeFuture = http::async_write(*stream, req, boost::asio::use_future);
    if (writeFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout) {
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
    if (stream != nullptr) {
        boost::beast::error_code ec;
        stream->shutdown(ec);
    }
}

std::string HttpStreamCommon::readRespAsStr(const std::shared_ptr<HttpStreamCommon::Stream> &stream, bool checkRespStatus) {
    boost::beast::basic_flat_buffer<std::allocator<uint8_t>> readBuffer;
    http::response_parser<http::string_body> parser;
    auto readHeaderFuture = http::async_read_header(*stream, readBuffer, parser, boost::asio::use_future);
    if (readHeaderFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout) {
        stream->next_layer().cancel();
        closeStream(stream);
        throw HttpException("Reading headers aborted on timeout");
    }
    readHeaderFuture.get();
    if (parser.get().result() != http::status::ok && checkRespStatus) {
        throw HttpException(
                "Bad status code " + std::to_string(static_cast<uint_fast32_t>(parser.get().result()))
        );
    }
    auto readFuture = http::async_read(*stream, readBuffer, parser, boost::asio::use_future);
    if (readFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout) {
        stream->next_layer().cancel();
        closeStream(stream);
        throw HttpException("Reading content aborted on timeout");
    }
    try {
        readFuture.get();
    } catch (const boost::system::system_error &ex) {
        if (ex.code() != boost::beast::http::error::need_buffer) {
            throw;
        }
    }
    auto res = parser.get().body();
    closeStream(stream);
    return res;
}

void HttpStreamCommon::readPartIntoBuffer(
        const std::shared_ptr<HttpStreamCommon::Stream> &stream,
        const std::string &uri,
        uint_fast32_t size,
        std::function<void(ByteVect)> succHandler,
        std::function<void(std::exception_ptr)> errHandler
) {
    auto readIntoBufFunc = [this, succHandler, size, stream, errHandler] (auto parser, auto readBuffer) {
        auto buf = std::make_shared<ByteVect>(size);
        parser->get().body().data = (&(*buf)[0]);
        parser->get().body().size = size;
        http::async_read(*stream, *readBuffer, *parser, createTimer(
                [this, succHandler, buf, stream, parser, readBuffer] (...) {
                    closeStream(stream);
                    succHandler(std::move(*buf));
                }, [this, stream, errHandler, succHandler, buf] (auto exc) {
                    closeStream(stream);
                    try {
                        std::rethrow_exception(exc);
                    } catch (const boost::system::system_error &ex) {
                        if (ex.code() != boost::beast::http::error::need_buffer) {
                            stream->next_layer().cancel();
                            errHandler(exc);
                        } else {
                            succHandler(std::move(*buf));
                        }
                    } catch (...) {
                        stream->next_layer().cancel();
                        errHandler(exc);
                    }
                },
                "Reading content aborted on timeout"
        ));
    };

    auto readHeaderFunc = [this, stream, readIntoBufFunc, errHandler, uri] {
        auto readBuffer = std::make_shared<boost::beast::basic_flat_buffer<std::allocator<uint8_t>>>();
        auto parser = std::make_shared<http::response_parser<http::buffer_body>>();
        http::async_read_header(*stream, *readBuffer, *parser, createTimer(
                [readIntoBufFunc, uri, parser, readBuffer] (...) {
                    if (parser->get().result() == http::status::range_not_satisfiable) {
                        throw WrongSizeException(uri);
                    }
                    if (parser->get().result() != http::status::partial_content && parser->get().result() != http::status::ok) {
                        throw HttpException(
                                "Bad status code " + std::to_string(static_cast<uint_fast32_t>(parser->get().result())) +
                                "when reading part"
                        );
                    }
                    readIntoBufFunc(parser, readBuffer);
                },
                [this, stream, errHandler] (auto exc) {
                    stream->next_layer().cancel();
                    closeStream(stream);
                    errHandler(exc);
                },
                "Reading headers aborted on timeout"
        ));
    };

    boost::asio::post(_ioc, readHeaderFunc);
}

ByteVect HttpStreamCommon::readIntoBuffer(
        const std::shared_ptr<HttpStreamCommon::Stream> &stream
) {
    boost::beast::basic_flat_buffer<std::allocator<uint8_t>> readBuffer;
    http::response_parser<http::vector_body<uint8_t>> parser;
    auto readHeaderFuture = http::async_read_header(*stream, readBuffer, parser, boost::asio::use_future);
    if (readHeaderFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout) {
        stream->next_layer().cancel();
        closeStream(stream);
        throw HttpException("Reading headers aborted on timeout");
    }
    readHeaderFuture.get();
    if (parser.get().result() != http::status::ok) {
        throw HttpException(
                "Bad status code " + std::to_string(static_cast<uint_fast32_t>(parser.get().result())) +
                "when reading part"
        );
    }

    auto readFuture = http::async_read(*stream, readBuffer, parser, boost::asio::use_future);
    if (readFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout) {
        stream->next_layer().cancel();
        closeStream(stream);
        throw HttpException("Reading content aborted on timeout");
    }
    try {
        readFuture.get();
    } catch (const boost::system::system_error &ex) {
        if (ex.code() != boost::beast::http::error::need_buffer) {
            throw;
        }
    }
    auto buf = std::move(parser.get().body());
    closeStream(stream);
    return buf;
}

void HttpStreamCommon::sendPartialGetReq(
        const std::shared_ptr<HttpStreamCommon::Stream> &stream,
        const HttpStreamCommon::HostPath &hostPath,
        const std::string &userAgent,
        uint_fast32_t byteStart,
        uint_fast32_t byteEnd,
        std::function<void()> succHandler,
        std::function<void(std::exception_ptr)> errHandler
) {
    auto writeFunc = [this, stream, succHandler, errHandler, hostPath, userAgent, byteStart, byteEnd] {
        auto req = std::make_shared<http::request<http::string_body>>(http::verb::get, hostPath.path, HttpStreamCommon::HTTP_VERSION);
        req->set(http::field::host, hostPath.host);
        req->set(http::field::range, "bytes=" + std::to_string(byteStart) + "-" + std::to_string(byteEnd));
        req->set(http::field::user_agent, userAgent);
        http::async_write(*stream, *req, createTimer(
                [succHandler, req](...) { succHandler(); }, [this, errHandler, stream](auto exc) {
                    stream->next_layer().cancel();
                    closeStream(stream);
                    errHandler(exc);
                }, "Partial get request aborted on timeout"
        ));
    };
    boost::asio::post(_ioc, writeFunc);
}

uint_fast32_t HttpStreamCommon::readSize(const std::shared_ptr<HttpStreamCommon::Stream> &stream) {
    http::response_parser<http::empty_body> parser;
    boost::beast::basic_flat_buffer<std::allocator<uint8_t>> readBuffer;
    parser.skip(true);
    auto readFuture = http::async_read(*stream, readBuffer, parser, boost::asio::use_future);
    if (readFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout) {
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

void HttpStreamCommon::sendPostReq(const std::shared_ptr<HttpStreamCommon::Stream> &stream,
                                   const HttpStreamCommon::HostPath &hostPath, const std::string &userAgent,
                                   const ByteVect &data, const std::string &contentType) {
    http::request<http::buffer_body> req{http::verb::post, hostPath.path, HttpStreamCommon::HTTP_VERSION};
    req.set(http::field::host, hostPath.host);
    req.set(http::field::user_agent, userAgent);
    req.set(http::field::content_type, contentType);
    req.set(http::field::content_length, data.size());
    req.body().data = (void *) &data[0];
    req.body().size = data.size();
    req.body().more = false;
    auto writeFuture = http::async_write(*stream, req, boost::asio::use_future);
    if (writeFuture.wait_for(std::chrono::milliseconds(_timeout)) == std::future_status::timeout) {
        stream->next_layer().cancel();
        closeStream(stream);
        throw HttpException("Post request aborted on timeout");
    }
    writeFuture.get();
}

std::string HttpStreamCommon::fieldsToPostReq(const std::unordered_map<std::string, std::string> &map) {
    std::ostringstream res;
    bool isFirst = true;
    for(const auto &item: map) {
        if (!isFirst) {
            res << "&";
        } else {
            isFirst = false;
        }
        res << item.first << "=" << uriEncode(item.second);
    }
    return res.str();
}

boost::asio::io_context &HttpStreamCommon::getIoContext() {
    return _ioc;
}
