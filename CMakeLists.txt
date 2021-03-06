cmake_minimum_required(VERSION 3.7)
project(vk_music_fs)

option(ENABLE_TESTS "Enable tests" OFF)

set(OLD_CXX_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_VERBOSE_MAKEFILE 1)
set(CMAKE_NO_SYSTEM_FROM_IMPORTED 1)

if(${ENABLE_TESTS})
    add_library(testlib INTERFACE)
    target_link_libraries(testlib INTERFACE gtest_main gmock)
    target_include_directories(testlib INTERFACE ${CURRENT_SRC_DIR})
    set(ALL_TESTS_SRC "")

    define_property(GLOBAL PROPERTY ALL_TESTS_SRC BRIEF_DOCS "All tests sources" FULL_DOCS "All tests sources")
    define_property(GLOBAL PROPERTY ALL_TESTS_LIBS BRIEF_DOCS "All libraries for tests" FULL_DOCS "All libraries for tests")
    set_property(GLOBAL PROPERTY ALL_TESTS_SRC "")
    set_property(GLOBAL PROPERTY ALL_TESTS_LIBS "")
endif()

macro(add_vmf_test name lib)
    if(${ENABLE_TESTS})
        add_executable(${name} ${ARGN})
        target_link_libraries(${name} PUBLIC testlib "${lib}_test")

        set(result "")
        foreach(test_file ${ARGN})
            get_filename_component(abs_path "${test_file}" REALPATH)
            list(APPEND result ${abs_path})
        endforeach()
        set_property(GLOBAL APPEND PROPERTY ALL_TESTS_SRC "${result}")
        set_property(GLOBAL APPEND PROPERTY ALL_TESTS_LIBS "${lib}_test")
    endif()
endmacro()

macro(add_vmf_lib name)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_library(${name} SHARED)
    else()
        add_library(${name} STATIC)
    endif()
    target_sources(${name} PRIVATE ${ARGN})
endmacro()

macro(add_vmf_lib_test name)
    add_vmf_lib(${name} ${ARGN})
    target_include_directories(${name} PRIVATE ${ROOT_DIR})

    if(${ENABLE_TESTS})
        add_library(${name}_test INTERFACE)
        target_link_libraries(${name}_test INTERFACE ${name})
        target_include_directories(${name}_test INTERFACE ${ROOT_DIR})
        target_include_directories(${name}_test INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
endmacro()

add_subdirectory(taglib_mod)
add_subdirectory(cfgpath)
add_subdirectory(json)
add_subdirectory(cpp-lru-cache-mod)
add_subdirectory(di)
add_subdirectory(nowide)
if(${ENABLE_TESTS})
    add_subdirectory(googletest)
endif()

set(CMAKE_CXX_FLAGS "${OLD_CXX_FLAGS} --std=gnu++17")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXX_FLAGS}")
if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
endif()

add_subdirectory(src)

if(${ENABLE_TESTS})
    get_property(ALL_TESTS_SRC GLOBAL PROPERTY ALL_TESTS_SRC)
    get_property(ALL_TESTS_LIBS GLOBAL PROPERTY ALL_TESTS_LIBS)
    add_executable(AllTests ${ALL_TESTS_SRC})
    target_link_libraries(AllTests PUBLIC testlib ${ALL_TESTS_LIBS})
endif()