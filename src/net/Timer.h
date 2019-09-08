#pragma once

#include "common_net.h"
#include "HttpException.h"
#include <boost/asio.hpp>
#include <variant>

namespace vk_music_fs {
    namespace net {
        template <typename TSuccFunc, typename TErrFunc>
        class Timer {
        public:
            Timer(
                    boost::asio::io_context &ioc,
                    uint_fast32_t time,
                    TSuccFunc succFunc,
                    TErrFunc errFunc,
                    const std::string &excString
            ): _succFunc(succFunc), _errFunc(errFunc), _excString(excString), _timer(ioc), _timeout(std::make_shared<std::atomic_bool>(false)) {
                _timer.expires_from_now(boost::posix_time::milliseconds(time));
                _timer.async_wait([timeout = _timeout] (...) {*timeout = true;});
            }

            Timer(Timer<TSuccFunc, TErrFunc> &&other) = default;

            template <typename ...TArgs>
            void operator()(const boost::system::error_code &ec, TArgs... val) {
                if (*_timeout) {
                    _errFunc(std::make_exception_ptr(HttpException{_excString}));
                } else {
                    _timer.cancel();
                    if (ec) {
                        _errFunc(std::make_exception_ptr(boost::system::system_error{ec}));
                    } else {
                        try {
                            _succFunc(std::forward<TArgs>(val)...);
                        } catch (...) {
                            _errFunc(std::current_exception());
                        }
                    }
                }
            }

            template <typename ...TArgs>
            void operator()(TArgs ...val) {
                if (*_timeout) {
                    _errFunc(std::make_exception_ptr(HttpException{_excString}));
                } else {
                    _timer.cancel();
                    try {
                        _succFunc(std::forward<TArgs>(val)...);
                    } catch (...) {
                        _errFunc(std::current_exception());
                    }
                }
            }
        private:
            boost::asio::deadline_timer _timer;
            TSuccFunc _succFunc;
            TErrFunc _errFunc;
            std::string _excString;
            std::shared_ptr<std::atomic_bool> _timeout;
        };
    }
}
