#pragma once

#include "RefreshAct.h"
#include "NumberAct.h"
#include <tuple>

namespace vk_music_fs{
    namespace fs{
        template <typename TFsUtils>
        using ActTuple = std::tuple<
                std::shared_ptr<RefreshAct<TFsUtils>>,
                std::shared_ptr<NumberAct<TFsUtils>>
        >;


        template <template <class> class TAct, typename TFsUtils>
        auto getAct(ActTuple<TFsUtils> &acts){
            return std::get<std::shared_ptr<TAct<TFsUtils>>>(acts);
        }
    }
}
