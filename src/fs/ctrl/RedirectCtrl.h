#pragma once

#include <fs/common_fs.h>
#include <fs/FsPath.h>

namespace vk_music_fs {
    namespace fs {
        template<typename TCtrl>
        class RedirectCtrl {
        public:
            RedirectCtrl(const std::shared_ptr<TCtrl> &ctrl) : _ctrl(ctrl){}

            template<typename... Args>
            auto getCtrlDir(Args&&... args){
                return _ctrl->getCtrlDir(std::forward<Args>(args)...);
            }

            template<typename... Args>
            auto transformPath(Args&&... args){
                return _ctrl->transformPath(std::forward<Args>(args)...);
            }

            template<typename... Args>
            auto checkCreateDirPath(Args&&... args){
                return _ctrl->checkCreateDirPath(std::forward<Args>(args)...);
            }

            template<typename... Args>
            auto setRootDir(Args&&... args){
                return _ctrl->setRootDir(std::forward<Args>(args)...);
            }

            template<typename... Args>
            auto createFile(Args&&... args){
                return _ctrl->createFile(std::forward<Args>(args)...);
            }

            template<typename... Args>
            auto supports(Args&&... args){
                return _ctrl->supports(std::forward<Args>(args)...);
            }

            template<typename... Args>
            auto createDir(Args&&... args){
                return _ctrl->createDir(std::forward<Args>(args)...);
            }

            template<typename... Args>
            auto rename(Args&&... args){
                return _ctrl->rename(std::forward<Args>(args)...);
            }

            template<typename... Args>
            auto getEntries(Args&&... args){
                return _ctrl->getEntries(std::forward<Args>(args)...);
            }

            template<typename... Args>
            auto getMeta(Args&&... args){
                return _ctrl->getMeta(std::forward<Args>(args)...);
            }

            template<typename... Args>
            auto deleteDir(Args&&... args){
                return _ctrl->deleteDir(std::forward<Args>(args)...);
            }

            template<typename... Args>
            auto deleteFile(Args&&... args){
                return _ctrl->deleteFile(std::forward<Args>(args)...);
            }

            template<typename... Args>
            auto open(Args&&... args){
                return _ctrl->open(std::forward<Args>(args)...);
            }

            template<typename... Args>
            auto getFileSize(Args&&... args){
                return _ctrl->getFileSize(std::forward<Args>(args)...);
            }

        private:
            std::shared_ptr<TCtrl> _ctrl;
        };
    }
}
