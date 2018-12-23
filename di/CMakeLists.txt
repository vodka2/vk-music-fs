project(di)

add_library(di INTERFACE)
target_sources(di INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/bindings/constructor_bindings.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/bindings/contextual_bindings.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/injections/assisted_injection.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/injections/concepts.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/injections/extensible_injector.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/injections/factory.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/injections/lazy.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/injections/named_parameters.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/injections/shared_factory.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/injections/xml_injection.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/policies/serialize.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/policies/types_dumper.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/policies/uml_dumper.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/providers/heap.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/providers/mocks_provider.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/scopes/scoped.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/scopes/session.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include/boost/di/extension/scopes/shared.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/boost/di.hpp
        )
target_include_directories(di INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/extension/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )