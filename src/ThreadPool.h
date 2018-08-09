#pragma once

#include <boost/asio.hpp>
#include <mutex>

namespace vk_music_fs {
    class ThreadPool {
    public:
        ThreadPool();
        template<typename T>
        void post(T func) {
            std::scoped_lock <std::mutex> lock(_mutex);
            boost::asio::post(_tp, func);
        }

    private:
        boost::asio::thread_pool _tp;
        std::mutex _mutex;
    };
}
