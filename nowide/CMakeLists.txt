project(nowide)

add_library(nowide
        src/iostream.cpp
        include/boost/nowide/integration/filesystem.hpp
        include/boost/nowide/args.hpp
        include/boost/nowide/cenv.hpp
        include/boost/nowide/config.hpp
        include/boost/nowide/convert.hpp
        include/boost/nowide/cstdio.hpp
        include/boost/nowide/cstdlib.hpp
        include/boost/nowide/filebuf.hpp
        include/boost/nowide/fstream.hpp
        include/boost/nowide/iostream.hpp
        include/boost/nowide/stackstring.hpp
        include/boost/nowide/system.hpp
        include/boost/nowide/utf8_codecvt.hpp
        include/boost/nowide/windows.hpp
        )
target_include_directories(nowide PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)