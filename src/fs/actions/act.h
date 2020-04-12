#pragma once

#include "RefreshAct.h"
#include "NumberAct.h"
#include "RemoveRefreshDirAct.h"
#include "DeleteDirAct.h"
#include "DeleteFileAct.h"
#include <tuple>

namespace vk_music_fs{
    namespace fs{

        template <typename TFsUtils>
        using ActTuple = std::tuple<
                std::shared_ptr<RefreshAct<TFsUtils>>,
                std::shared_ptr<NumberAct<TFsUtils>>,
                std::shared_ptr<RemoveRefreshDirAct>,
                std::shared_ptr<DeleteDirAct<TFsUtils>>,
                std::shared_ptr<DeleteFileAct<TFsUtils>>
        >;


        template <template <class> class TAct, typename TFsUtils>
        auto getAct(ActTuple<TFsUtils> &acts){
            return std::get<std::shared_ptr<TAct<TFsUtils>>>(acts);
        }

        template <class TAct, typename TTuple>
        auto getAct(TTuple &acts){
            return std::get<std::shared_ptr<TAct>>(acts);
        }
    }
}
