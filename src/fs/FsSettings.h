#pragma once

#include <common/common.h>
#include "common_fs.h"

namespace vk_music_fs {
    namespace fs {
        class FsSettings {
        public:
            FsSettings(
                    const NumSearchFiles &numSearchFiles,
                    const Mp3Extension &mp3Ext,
                    const CreateDummyDirs &createDummyDirs,
                    const PathToFs &pathToFs,
                    const UseAsyncNotifier &useAsyncNotifier
            );

            uint_fast32_t getNumSearchFiles() const;

            const std::string getMp3Ext() const;

            bool isCreateDummyDirs();

            const std::string getPathToFs() const;

            bool isUseAsyncNotifier();

        private:
            uint_fast32_t _numSearchFiles;
            std::string _mp3Ext;
            std::string _pathToFs;
            CreateDummyDirs _createDummyDirs;
            UseAsyncNotifier _useAsyncNotifier;
        };
    }
}
