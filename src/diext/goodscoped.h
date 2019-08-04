#pragma once
#include <boost/di.hpp>
#include <boost/di/extension/scopes/scoped.hpp>
#include <unordered_map>
#include "Storage.h"

BOOST_DI_NAMESPACE_BEGIN
namespace extension {
    namespace detail {
        class goodscoped {
        public:
            template <class, class T>
            class scope {
            public:
                template <class T_, class>
                using is_referable = typename wrappers::shared<scoped, T>::template is_referable<T_>;

                scope &operator=(scope &&other) noexcept {
                    this->object_ = other.object_;
                    other.object_ = nullptr;
                    return *this;
                }

                template <class, class, class TProvider, class T_ = aux::decay_t<decltype(aux::declval<TProvider>().get())>>
                static decltype(wrappers::shared<scoped, T_>{std::shared_ptr<T_>{std::shared_ptr<T_>{aux::declval<TProvider>().get()}}})
                try_create(const TProvider &);

                template <class T_, class, class TProvider>
                auto create(const TProvider &provider) {
                    return create_impl<aux::decay_t<decltype(provider.get())>>(provider);
                }

                scope() = default;

            private:
                template <class, class TProvider>
                auto create_impl(const TProvider &provider) {
                    auto& injector = provider.super();
                    std::shared_ptr<Storage> storage = injector.template create<std::shared_ptr<Storage>>();
                    std::shared_ptr<std::shared_ptr<T>> obj;
                    if(storage->objects.count(getTypeId<T>()) != 0){
                        obj = std::static_pointer_cast<std::shared_ptr<T>>(storage->objects[getTypeId<T>()]);
                    } else {
                        obj = std::make_shared<std::shared_ptr<T>>(std::shared_ptr<T>{provider.get()});
                        storage->objects.insert(std::make_pair<>(getTypeId<T>(), obj));
                    }
                    return wrappers::shared<scoped, T, std::shared_ptr<T>&>{*obj};
                }
            };
        };
    }  // namespace detail

    using goodscoped = detail::goodscoped;

}  // namespace extension
BOOST_DI_NAMESPACE_END
