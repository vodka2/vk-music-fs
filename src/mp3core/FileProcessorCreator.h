#pragma once


#include <common/common.h>
#include <diext/ext_factory.hpp>
#include "RemoteFile.h"

namespace vk_music_fs {
    template<typename TFileProcessor>
    class FileProcessorCreator {
    public:
        typedef TFileProcessor Processor;
        FileProcessorCreator(std::shared_ptr<boost::di::extension::iextfactory <
                             TFileProcessor,
                             SongData,
                             RemoteFileUri,
                             TagSize,
                             RemoteFile,
                             CachedFilename
                             >> fact): _fact(fact) {

        }

        template <typename TCache>
        auto create(const RemoteFile &remFile, const std::shared_ptr<TCache> &cache, const CachedFilename &fname) {
            return _fact->createShared(
                    SongData{remFile.getSongData()},
                    RemoteFileUri{remFile.getUri()},
                    TagSize{cache->getTagSize(remFile)},
                    RemoteFile(remFile),
                    fname);
        }
    private:
        std::shared_ptr<boost::di::extension::iextfactory <
                TFileProcessor,
                SongData,
                RemoteFileUri,
                TagSize,
                RemoteFile,
                CachedFilename
        >> _fact;
    };
}