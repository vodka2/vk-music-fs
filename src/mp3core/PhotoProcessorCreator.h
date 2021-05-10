#pragma once


#include <common/common.h>
#include <diext/ext_factory.hpp>
#include <common/RemotePhotoFile.h>
#include "RemoteFile.h"

namespace vk_music_fs {
    template<typename TPhotoProcessor>
    class PhotoProcessorCreator {
    public:
        typedef TPhotoProcessor Processor;
        PhotoProcessorCreator(std::shared_ptr<boost::di::extension::iextfactory <
                             TPhotoProcessor,
                             RemotePhotoFile,
                             RemoteFileUri,
                             CachedFilename
                             >> fact): _fact(fact) {
        }

        template <typename TCache>
        auto create(const RemotePhotoFile &remFile, const std::shared_ptr<TCache> &, const CachedFilename &fname) {
            return _fact->createShared(remFile, RemoteFileUri{remFile.getUri()}, fname);
        }
    private:
        std::shared_ptr<boost::di::extension::iextfactory <
                TPhotoProcessor,
                RemotePhotoFile,
                RemoteFileUri,
                CachedFilename
        >> _fact;
    };
}