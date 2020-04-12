#pragma once

#include <fs/ctrl/ThrowExCtrl.h>

namespace vk_music_fs {
    namespace fs {
        class CtrlM0 : public ThrowExCtrl {
        public:
            template<typename... T>
            CtrlM0(T &&... args) {} //NOLINT
            MOCK_CONST_METHOD2(rename, void(FsPath&, FsPath&));
            MOCK_CONST_METHOD0(getDirName, std::string());
            MOCK_CONST_METHOD0(getCtrlDir, DirPtr());
        };
    }
}

typedef testing::NiceMock<vk_music_fs::fs::CtrlM0> CtrlM;