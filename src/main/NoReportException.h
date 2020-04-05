#pragma once

#include <stdexcept>

namespace vk_music_fs {
    class NoReportException: public std::runtime_error {
    public:
        NoReportException();
    };
}