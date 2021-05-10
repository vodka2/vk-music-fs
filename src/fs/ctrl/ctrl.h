#include <tuple>
#include <memory>
#include "DummyDirWrapper.h"
#include "RemoteFileWrapper.h"
#include "SingleDirCtrl.h"
#include "MyAudiosCtrl.h"
#include "SearchSongNameCtrl.h"
#include "SearchSongNameArtistHelper.h"
#include "SearchSongNameSongHelper.h"
#include "PlaylistCtrl.h"
#include "SimilarCtrl.h"
#include "AddToMyAudiosCtrl.h"
#include "RootCtrl.h"

namespace vk_music_fs {
    namespace fs {
        template<typename TFsUtils, typename TFileObtainer, typename TFileManager,
                typename TPhotoManager, typename TAsyncFsManager>
        using CtrlTuple = std::tuple<
                std::shared_ptr<
                        DummyDirWrapper<
                                AddToMyAudiosCtrl<
                                        SimilarCtrl<
                                                RemoteFileWrapper<
                                                        SingleDirCtrl<
                                                                MyAudiosCtrl<TFsUtils, TFileObtainer, TAsyncFsManager>,
                                                                TFsUtils
                                                        >,
                                                        TFsUtils,
                                                        TFileManager,
                                                        TPhotoManager
                                                >,
                                                TFsUtils,
                                                TFileObtainer,
                                                TAsyncFsManager
                                        >,
                                        TFsUtils,
                                        TFileObtainer
                                >,
                                TFsUtils
                        >
                >,
                std::shared_ptr<
                        DummyDirWrapper<
                                AddToMyAudiosCtrl<
                                        SimilarCtrl<
                                                RemoteFileWrapper<
                                                        SingleDirCtrl<
                                                                SearchSongNameCtrl<
                                                                        TFsUtils, TFileObtainer,
                                                                        SearchSongNameArtistHelper, TAsyncFsManager
                                                                >,
                                                                TFsUtils
                                                        >,
                                                        TFsUtils,
                                                        TFileManager,
                                                        TPhotoManager
                                                >,
                                                TFsUtils,
                                                TFileObtainer,
                                                TAsyncFsManager
                                        >,
                                        TFsUtils,
                                        TFileObtainer
                                >,
                                TFsUtils
                        >
                >,
                std::shared_ptr<
                        DummyDirWrapper<
                                AddToMyAudiosCtrl<
                                        SimilarCtrl<
                                                RemoteFileWrapper<
                                                        SingleDirCtrl<
                                                                SearchSongNameCtrl<
                                                                        TFsUtils,
                                                                        TFileObtainer,
                                                                        SearchSongNameSongHelper, TAsyncFsManager
                                                                >,
                                                                TFsUtils
                                                        >,
                                                        TFsUtils,
                                                        TFileManager,
                                                        TPhotoManager
                                                >,
                                                TFsUtils,
                                                TFileObtainer,
                                                TAsyncFsManager
                                        >,
                                        TFsUtils,
                                        TFileObtainer
                                >,
                                TFsUtils
                        >
                >,
                std::shared_ptr<
                        DummyDirWrapper<
                                AddToMyAudiosCtrl<
                                        SimilarCtrl<
                                                RemoteFileWrapper<
                                                        SingleDirCtrl<
                                                                PlaylistCtrl<TFsUtils, TFileObtainer, TAsyncFsManager>,
                                                                TFsUtils
                                                                >,
                                                        TFsUtils,
                                                        TFileManager,
                                                        TPhotoManager
                                                >,
                                                TFsUtils,
                                                TFileObtainer,
                                                TAsyncFsManager
                                        >,
                                        TFsUtils,
                                        TFileObtainer
                                >,
                                TFsUtils
                        >
                >,
                std::shared_ptr<RootCtrl<TFsUtils>>
        >;
    }
}