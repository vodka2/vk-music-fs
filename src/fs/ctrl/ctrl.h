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
#include "RootCtrl.h"

namespace vk_music_fs {
    namespace fs {
        template<typename TFsUtils, typename TQueryMaker, typename TAsyncFsManager>
        class MyAudiosCtrl;

        template<typename TFsUtils, typename TQueryMaker, typename THelper, typename TAsyncFsManager>
        class SearchSongNameCtrl;

        template<typename TCtrl, typename TFsUtils>
        class DummyDirWrapper;

        template<typename TCtrl, typename TFsUtils>
        class SingleDirCtrl;

        template <typename TFsUtils>
        class RootCtrl;

        template <typename TCtrl, typename TFsUtils, typename TFileManager>
        class RemoteFileWrapper;

        class SearchSongNameSongHelper;

        class SearchSongNameArtistHelper;

        template<typename TFsUtils, typename TFileObtainer, typename TFileManager, typename TAsyncFsManager>
        using CtrlTuple = std::tuple<
                std::shared_ptr<
                        DummyDirWrapper<
                                SimilarCtrl<
                                        RemoteFileWrapper<
                                                SingleDirCtrl<
                                                        MyAudiosCtrl<TFsUtils, TFileObtainer, TAsyncFsManager>,
                                                        TFsUtils
                                                >,
                                                TFsUtils,
                                                TFileManager
                                        >,
                                        TFsUtils,
                                        TFileObtainer,
                                        TAsyncFsManager
                                >,
                                TFsUtils
                        >
                >,
                std::shared_ptr<
                        DummyDirWrapper<
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
                                                TFileManager
                                        >,
                                        TFsUtils,
                                        TFileObtainer,
                                        TAsyncFsManager
                                >,
                                TFsUtils
                        >
                >,
                std::shared_ptr<
                        DummyDirWrapper<
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
                                                TFileManager
                                        >,
                                        TFsUtils,
                                        TFileObtainer,
                                        TAsyncFsManager
                                >,
                                TFsUtils
                        >
                >,
                std::shared_ptr<
                        DummyDirWrapper<
                                SimilarCtrl<
                                        RemoteFileWrapper<
                                                SingleDirCtrl<
                                                        PlaylistCtrl<TFsUtils, TFileObtainer, TAsyncFsManager>,
                                                        TFsUtils
                                                        >,
                                                TFsUtils,
                                                TFileManager
                                        >,
                                        TFsUtils,
                                        TFileObtainer,
                                        TAsyncFsManager
                                >,
                                TFsUtils
                        >
                >,
                std::shared_ptr<RootCtrl<TFsUtils>>
        >;
    }
}