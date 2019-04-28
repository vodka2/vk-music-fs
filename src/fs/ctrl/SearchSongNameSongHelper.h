#pragma once

#include <fs/common_fs.h>

namespace vk_music_fs {
    namespace fs {
        class SearchSongNameSongHelper {
        public:
            template <typename TFileObtainer>
            std::vector<RemoteFile> searchFiles(
                    const std::shared_ptr<TFileObtainer> &obtainer,
                    const std::string &name, uint_fast32_t offset, uint_fast32_t cnt
            ){
                return obtainer->searchBySongName(name, offset, cnt);
            }

            std::string getDirName(){
                return "Search";
            }
        };
    }
}
