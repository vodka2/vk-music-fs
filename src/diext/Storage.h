#pragma once

#include <boost/di.hpp>
#include <unordered_map>

BOOST_DI_NAMESPACE_BEGIN
namespace extension {
    using TypeId = uintptr_t;

    template<typename T>
    static TypeId getTypeId() {
        static uint32_t placeHolder;
        return (reinterpret_cast<TypeId>(&placeHolder));
    }

    class Storage {
    public:
        Storage() {}

        std::unordered_map<TypeId, std::shared_ptr<void>> objects;
    };

}
BOOST_DI_NAMESPACE_END
