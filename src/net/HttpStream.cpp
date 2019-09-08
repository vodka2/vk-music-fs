#include "WrongSizeException.h"
#include "HttpStream.h"
#include "HttpException.h"
#include "Timer.h"

using namespace vk_music_fs;
using namespace net;

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace ssl = boost::asio::ssl;

uint_fast32_t HttpStream::doRead(uint_fast32_t maxSize) {
    try {
        std::promise<uint_fast32_t> prom;
        auto errFunc = [&prom] (auto exc) {
            prom.set_exception(exc);
        };
        auto succFunc = [this, &prom] (auto size) {
            if (_parser.is_done()) {
                close();
            }
            prom.set_value(size);
        };
        auto getSizeFunc = [this, succFunc, errFunc, maxSize] {
            http::async_read(*_stream, _readBuffer, _parser, _common->createTimer(
                    [this, succFunc] (uint_fast32_t size) {
                        succFunc(size);
                    },
                    [errFunc, maxSize, succFunc] (auto exc) {
                        try {
                            std::rethrow_exception(exc);
                        } catch (const boost::system::system_error &ex) {
                            if(ex.code() != boost::beast::http::error::need_buffer) {
                                errFunc(std::current_exception());
                            } else {
                                succFunc(maxSize);
                            }
                        } catch (...) {
                            errFunc(std::current_exception());
                        }
                    },
                    "Reading aborted on timeout"
            ));
        };
        boost::asio::post(_common->getIoContext(), getSizeFunc);
        return prom.get_future().get();
    } catch (const boost::system::system_error &ex) {
        close();
        throw HttpException(std::string("Error reading uri ") + _uri + ". " + ex.what());
    } catch (const HttpException &ex) {
        close();
        throw;
    }
}

ByteVect HttpStream::read(uint_fast32_t offset, uint_fast32_t length) {
    try {
        if (offset >= _size || _closed) {
            return {};
        }
        std::promise<ByteVect> prom;
        auto errFunc = [&prom] (auto exc) {
            prom.set_exception(exc);
        };
        auto maxReadByte = std::min(offset + length - 1, _size - 1);
        auto readSize = maxReadByte + 1 - offset;

        auto readPartFunc = [this, readSize, &prom, errFunc] (auto stream) {
            _common->readPartIntoBuffer(stream, _uri, readSize, [&prom] (auto vect) {
                prom.set_value(std::move(vect));
            }, errFunc);
        };

        _common->connect(_hostPath, [this, offset, errFunc, readPartFunc, maxReadByte] (auto strm) {
            _common->sendPartialGetReq(
                    strm, _hostPath, _userAgent, offset, maxReadByte,
                    [strm, readPartFunc]{
                        readPartFunc(std::move(strm));
                    },
                    errFunc
            );
        }, errFunc);

        return prom.get_future().get();
    } catch (const boost::system::system_error &ex) {
        throw HttpException(std::string("Error reading part of uri ") + _uri + ". " + ex.what());
    } catch (const HttpException &ex) {
        close();
        throw;
    }
}

HttpStream::HttpStream(
        const Mp3Uri &uri, const std::shared_ptr<HttpStreamCommon> &common, const UserAgent &userAgent,
        const HttpTimeout &timeout
)
: _timeout(timeout.t), _closed(false),
  _uri(uri.t), _userAgent(userAgent.t), _common(common), _hostPath(_common->getHostPath(uri)) {
    _parser.body_limit(std::numeric_limits<std::uint_fast32_t>::max());
}

void HttpStream::open(uint_fast32_t offset, uint_fast32_t totalSize) {
    try {
        std::promise<void> prom;
        auto errFunc = [&prom] (auto exc) {
            prom.set_exception(exc);
        };
        auto processHeaderFunc = [this, &prom, offset] (...) {
            if (
                    (offset == 0 && _parser.get().result() != http::status::ok) ||
                    (offset != 0 && _parser.get().result() != http::status::partial_content)
            ) {
                throw HttpException(
                        "Bad status code " + std::to_string(static_cast<uint_fast32_t>(_parser.get().result())) +
                        " when opening " + _uri + " when reading");
            }
            if ((_size - offset) != static_cast<uint_fast32_t>(*_parser.content_length())) {
                close();
                throw WrongSizeException(
                        _size - offset, static_cast<uint_fast32_t>(*_parser.content_length()),
                        _uri
                );
            }
            prom.set_value();
        };
        auto readHeadersFunc = [this, processHeaderFunc, errFunc] {
            http::async_read_header(*_stream, _readBuffer, _parser, _common->createTimer(
                    processHeaderFunc, [this, errFunc] (auto exc) {
                        _stream->next_layer().cancel();
                        errFunc(exc);
                    }, "Reading headers aborted on timeout"
            ));
        };
        _common->connect(_hostPath, [this, totalSize, offset, errFunc, readHeadersFunc] (auto strm) {
            _stream = strm;
            _size = totalSize;
            if(offset == 0) {
                _common->sendGetReq(_stream, _hostPath, _userAgent, readHeadersFunc, errFunc);
            } else {
                _common->sendPartialGetReq(_stream, _hostPath, _userAgent, offset, _size - 1, readHeadersFunc, errFunc);
            }
        }, errFunc);

        prom.get_future().get();
    } catch (const boost::system::system_error &ex) {
        close();
        throw HttpException(std::string("Error opening uri ")  + _uri + ". " + ex.what());
    } catch (const HttpException &ex) {
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
