#pragma once

namespace vk_music_fs {
    namespace fs {
        class ThreadPoolM0 {
        public:
            template<typename... T>
            ThreadPoolM0(T &&... args) {} //NOLINT
            template <typename T>
            void post(T func) { //NOLINT
                func();
            }
        };
    }
}

typedef testing::NiceMock<vk_music_fs::fs::ThreadPoolM0> ThreadPoolM;