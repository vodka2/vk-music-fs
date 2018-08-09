#pragma once

#include <string>
#include <fstream>
#include <common.h>
#include <mutex>
#include <atomic>

namespace vk_music_fs {
    class MusicFile {
    public:
        explicit MusicFile(const std::string &name);

        void write(ByteVect vect);

        ByteVect read(uint_fast32_t offset, uint_fast32_t size);

        void finish();

        void close();

        uint_fast32_t getSize();

    private:
        std::string _name;
        std::mutex _mutex;
        std::fstream _fs;
        std::atomic_uint_fast32_t _size;
        std::atomic_bool _finished;
    };
}
