//
// Copyright (c) 2012-2018 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <memory>

#include "boost/di.hpp"
#include "boost/di/extension/injections/extensible_injector.hpp"

BOOST_DI_NAMESPACE_BEGIN
namespace extension {

template <class T, class... TArgs>
struct iextfactory {
  virtual ~iextfactory() noexcept = default;
  virtual std::shared_ptr<T> createShared(TArgs...) const = 0;
};

template <class, class, class>
struct extfactory_impl;

template <class TInjector, class T, class I, class... TArgs>
struct extfactory_impl<TInjector, T, iextfactory<I, TArgs...>> : iextfactory<I, TArgs...> {
  explicit extfactory_impl(const TInjector& injector) : injector_(const_cast<TInjector&>(injector)) {}

  std::shared_ptr<I> createShared(TArgs... args) const override {
    // clang-format off
    auto injector = make_injector(
      make_extensible(injector_)
#if (__clang_major__ == 3) && (__clang_minor__ > 4) || defined(__GCC___) || defined(__MSVC__)
      , bind<TArgs>().to(std::forward<TArgs>(args))[override]...
#else // wknd for clang 3.4
      , core::dependency<scopes::instance, TArgs, TArgs, no_name, core::override>(std::forward<TArgs>(args))...
#endif
    );
    // clang-format on

    return injector.template create<std::shared_ptr<T>>();
  }

 private:
  TInjector& injector_;
};

template <class T>
struct extfactory {
  template <class TInjector, class TDependency>
  auto operator()(const TInjector& injector, const TDependency&) const {
    static auto sp = std::make_shared<extfactory_impl<TInjector, T, typename TDependency::expected>>(injector);
    return sp;
  }
};

}  // namespace extension
BOOST_DI_NAMESPACE_END

