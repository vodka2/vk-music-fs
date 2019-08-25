#pragma once
#include <boost/asio.hpp>
#include <gmock/gmock.h>
#include <common/common.h>

class ThreadPoolM{
public:
    boost::asio::thread_pool tp;
    std::mutex mutex;
    ThreadPoolM(){}; //NOLINT
    template <typename T>
    void post(T func){ //NOLINT
        std::scoped_lock<std::mutex> lock(mutex);
        boost::asio::post(tp, func);
    }
};
