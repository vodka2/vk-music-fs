project(di)

add_library(di
        extension/include/boost/di/extension/bindings/constructor_bindings.hpp
        extension/include/boost/di/extension/bindings/contextual_bindings.hpp
        extension/include/boost/di/extension/injections/assisted_injection.hpp
        extension/include/boost/di/extension/injections/concepts.hpp
        extension/include/boost/di/extension/injections/extensible_injector.hpp
        extension/include/boost/di/extension/injections/factory.hpp
        extension/include/boost/di/extension/injections/lazy.hpp
        extension/include/boost/di/extension/injections/named_parameters.hpp
        extension/include/boost/di/extension/injections/shared_factory.hpp
        extension/include/boost/di/extension/injections/xml_injection.hpp
        extension/include/boost/di/extension/policies/serialize.hpp
        extension/include/boost/di/extension/policies/types_dumper.hpp
        extension/include/boost/di/extension/policies/uml_dumper.hpp
        extension/include/boost/di/extension/providers/heap.hpp
        extension/include/boost/di/extension/providers/mocks_provider.hpp
        extension/include/boost/di/extension/scopes/scoped.hpp
        extension/include/boost/di/extension/scopes/session.hpp
        extension/include/boost/di/extension/scopes/shared.hpp
        include/boost/di.hpp
        )
target_include_directories(di PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )
set_target_properties(di PROPERTIES LINKER_LANGUAGE CXX)