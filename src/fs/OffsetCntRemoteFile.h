#pragma once

#include <mp3core/RemoteFile.h>
#include "common_fs.h"

namespace vk_music_fs {
    namespace fs {
        class OffsetCntRemoteFile {
        public:
            uint_fast32_t getOffset() const;

            void setOffset(uint_fast32_t offset);

            uint_fast32_t getCnt() const;

            void setCnt(uint_fast32_t cnt);

            DirPtr getCounterDir() const;

            void setCounterDir(const DirWPtr &counterDir);

            OffsetCntRemoteFile(uint_fast32_t offset, uint_fast32_t cnt, const DirWPtr &counterDir, const RemoteFileId &remoteFileId);

            RemoteFileId getRemoteFileId() const;

            void setRemoteFileId(const RemoteFileId &remoteFileId);

        private:
            uint_fast32_t _offset;
            uint_fast32_t _cnt;
            DirWPtr _counterDir;
            RemoteFileId _remoteFileId;
        };
    }
}
