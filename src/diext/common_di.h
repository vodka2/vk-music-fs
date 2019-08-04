#pragma once

#include <boost/di.hpp>
#include <memory>
#include "goodscoped.h"

namespace vk_music_fs{
    template <typename T>
    class InjPtr: public std::shared_ptr<T>{
    public:
        InjPtr(std::shared_ptr<T> t): std::shared_ptr<T>(t){} //NOLINT
    };

    template <typename ...Ts>
    auto makeInjPtr(Ts... data){
        typedef decltype(boost::di::make_injector(std::forward<Ts>(data)...)) InjType;
        return InjPtr<InjType>{std::make_shared<InjType>(boost::di::make_injector(std::forward<Ts>(data)...))};
    }

    class CustomScopePolicy {
    public:
        template <class T>
        auto provider(T*) noexcept {
            return boost::di::providers::stack_over_heap{};
        }
        template <class T>
        auto policies(T*) noexcept {
            return boost::di::make_policies();
        }

        template <class T>
        struct scope_traits {
            using type = boost::di::scopes::unique;
        };

        template <class T>
        struct scope_traits<const std::shared_ptr<T>&> {
            using type = boost::di::extension::goodscoped;
        };
        template <class... T>
        struct scope_traits<const std::tuple<T...>&> {
            using type = boost::di::scopes::unique;
        };
        template <class T>
        struct scope_traits<T&> {
            using type = boost::di::scopes::singleton;
        };
        template <class T>
        struct scope_traits<std::shared_ptr<T>> {
            using type = boost::di::extension::goodscoped;
        };


        template <class T>
        using memory_traits = boost::di::type_traits::memory_traits<T>;
    };

    template <typename ...Ts>
    auto makeStorageInj(Ts... data){
        return boost::di::make_injector<CustomScopePolicy>(
                boost::di::bind<boost::di::extension::Storage>.in(boost::di::extension::scoped),
                std::forward<Ts>(data)...
        );
    }
}
