project(mp3core)

add_vmf_lib_test(main
        Application.cpp
        Application.h
        ProgramOptions.cpp
        ProgramOptions.h
        NoReportException.cpp
        NoReportException.h
        FakeFs.h
)

target_link_libraries(
        main
        common json
        tag ${ZLIB_LIBRARY_RESULT} cpp-lru-cache di cfgpath
)