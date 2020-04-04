#pragma once

namespace vk_music_fs {
    namespace fs {
        class RealFsM0 {
        public:
            template<typename... T>
            RealFsM0(T &&... args) {} //NOLINT
            MOCK_CONST_METHOD1(createFile, void(
                    const std::string&));
        };
    }
}

typedef testing::NiceMock<vk_music_fs::fs::RealFsM0> RealFsM;
