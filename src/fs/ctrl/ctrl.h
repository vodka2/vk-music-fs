#include <tuple>

namespace vk_music_fs {
    namespace fs {
        template<typename TFsUtils, typename TQueryMaker>
        class MyAudiosCtrl;

        template<typename TFsUtils, typename TQueryMaker, typename THelper>
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

        template<typename TFsUtils, typename TFileObtainer, typename TFileManager>
        using CtrlTuple = std::tuple<
                std::shared_ptr<
                        DummyDirWrapper<
                                RemoteFileWrapper<
                                        SingleDirCtrl<
                                                MyAudiosCtrl<TFsUtils, TFileObtainer>,
                                                TFsUtils
                                        >,
                                        TFsUtils,
                                        TFileManager
                                >,
                                TFsUtils
                        >
                >,
                std::shared_ptr<
                        DummyDirWrapper<
                                RemoteFileWrapper<
                                        SingleDirCtrl<
                                                SearchSongNameCtrl<TFsUtils, TFileObtainer, SearchSongNameArtistHelper>,
                                                TFsUtils
                                        >,
                                        TFsUtils,
                                        TFileManager
                                >,
                                TFsUtils
                        >
                >,
                std::shared_ptr<
                        DummyDirWrapper<
                                RemoteFileWrapper<
                                        SingleDirCtrl<
                                                SearchSongNameCtrl<TFsUtils, TFileObtainer, SearchSongNameSongHelper>,
                                                TFsUtils
                                        >,
                                        TFsUtils,
                                        TFileManager
                                >,
                                TFsUtils
                        >
                >,
                std::shared_ptr<RootCtrl<TFsUtils>>
        >;
    }
}