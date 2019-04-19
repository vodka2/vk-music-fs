#pragma once

#include <common.h>

namespace vk_music_fs {
    namespace fs {
        class FsSettings {
        public:
            FsSettings(
                    const NumSearchFiles &numSearchFiles,
                    const Mp3Extension &mp3Ext,
                    const CreateDummyDirs &createDummyDirs
            );

            uint_fast32_t getNumSearchFiles() const;

            const std::string getMp3Ext() const;

            bool isCreateDummyDirs() const;

        private:
            uint_fast32_t _numSearchFiles;
            std::string _mp3Ext;
            bool _createDummyDirs;
        };
    }
}
