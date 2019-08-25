project(token)

add_vmf_lib_test(token

        common_token.h
        ProtobufException.cpp
        ProtobufException.h
        SmallProtobufHelper.cpp
        SmallProtobufHelper.h
        TokenException.cpp
        TokenException.h
        TokenReceiver.cpp
        TokenReceiver.h
)

target_link_libraries(
        token
        net common
        json
)

