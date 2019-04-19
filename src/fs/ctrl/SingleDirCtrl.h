#pragma once

#include <boost/algorithm/string/predicate.hpp>
#include <fs/IdGenerator.h>
#include "RedirectCtrl.h"
#include <fs/OffsetCnt.h>

namespace vk_music_fs {
    namespace fs {
        template <typename TCtrl, typename TFsUtils>
        class SingleDirCtrl: public RedirectCtrl<TCtrl> {
        public:
            SingleDirCtrl(
                    const std::shared_ptr<TFsUtils> &utils,
                    const std::shared_ptr<TCtrl> &ctrl
            ) : RedirectCtrl<TCtrl>(ctrl), _fsUtils(utils), _ctrl(ctrl){
            }

            std::string transformPath(const std::string &path){
                return _fsUtils->stripPathPrefix(path, _ctrl->getDirName());
            }

            bool supports(const std::string &path) {
                return boost::starts_with(path, std::string("/") + _ctrl->getDirName());
            }
        private:
            std::shared_ptr<TFsUtils> _fsUtils;
            std::shared_ptr<TCtrl> _ctrl;
        };
    }
}
