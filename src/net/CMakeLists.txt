project(net)

add_vmf_lib_test(net

        HttpException.cpp
        HttpException.h
        HttpStream.cpp
        HttpStream.h
        HttpStreamCommon.cpp
        HttpStreamCommon.h
        Mp3SizeObtainer.cpp
        Mp3SizeObtainer.h
        VkApiQueryMaker.cpp
        VkApiQueryMaker.h
        WrongSizeException.cpp
        WrongSizeException.h
)

target_link_libraries(
        net common
        Boost::filesystem Boost::thread Boost::program_options Boost::system Threads::Threads OpenSSL::SSL OpenSSL::Crypto
        ${EXTRA_NET_LIBS}
)

