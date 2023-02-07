include(ExternalProject)

set(BORINGSSL_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/uws/uSockets/boringssl")
set(BORINGSSL_INCLUDE_DIRS ${BORINGSSL_SOURCE_DIR}/include)
set(BORINGSSL_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/boringssl.build)
set(BORINGSSL_STATIC_LIBRARIES
    ${BORINGSSL_BINARY_DIR}/ssl/libssl.a
    ${BORINGSSL_BINARY_DIR}/crypto/libcrypto.a
    #${BORINGSSL_BINARY_DIR}/decrepit/libdecrepit.a
)

ExternalProject_Add(boringssl
    PREFIX ${BORINGSSL_BINARY_DIR}/prefix
    SOURCE_DIR ${BORINGSSL_SOURCE_DIR}
    CONFIGURE_COMMAND ${CMAKE_COMMAND}
        -G ${CMAKE_GENERATOR}
        -S ${BORINGSSL_SOURCE_DIR}
        -B ${BORINGSSL_BINARY_DIR}
        -DFUZZ=OFF
        -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=${CMAKE_POSITION_INDEPENDENT_CODE}
        -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}

    BUILD_COMMAND ${CMAKE_COMMAND}
        --build ${BORINGSSL_BINARY_DIR}
        --target libcrypto.a --target libssl.a

    BUILD_ALWAYS TRUE
    BUILD_BYPRODUCTS
        ${BORINGSSL_STATIC_LIBRARIES}

    INSTALL_COMMAND ""

    USES_TERMINAL_CONFIGURE TRUE
    USES_TERMINAL_BUILD TRUE
)

add_library(uws STATIC
    uws/uSockets/src/bsd.c
    uws/uSockets/src/context.c
    uws/uSockets/src/libusockets.h
    uws/uSockets/src/loop.c
    uws/uSockets/src/quic.c
    uws/uSockets/src/quic.h
    uws/uSockets/src/socket.c
    uws/uSockets/src/udp.c

    uws/uSockets/src/crypto/openssl.c
    uws/uSockets/src/crypto/sni_tree.cpp

    uws/uSockets/src/eventing/epoll_kqueue.c

    uws/uSockets/src/internal/internal.h
    uws/uSockets/src/internal/loop_data.h

    uws/src/App.h
    uws/src/AsyncSocket.h
    uws/src/AsyncSocketData.h
    uws/src/BloomFilter.h
    uws/src/ChunkedEncoding.h
    uws/src/ClientApp.h
    uws/src/Http3App.h
    uws/src/Http3Context.h
    uws/src/Http3ContextData.h
    uws/src/Http3Request.h
    uws/src/Http3Response.h
    uws/src/Http3ResponseData.h
    uws/src/HttpContext.h
    uws/src/HttpContextData.h
    uws/src/HttpParser.h
    uws/src/HttpResponse.h
    uws/src/HttpResponseData.h
    uws/src/HttpRouter.h
    uws/src/Loop.h
    uws/src/LoopData.h
    uws/src/MessageParser.h
    uws/src/MoveOnlyFunction.h
    uws/src/Multipart.h
    uws/src/PerMessageDeflate.h
    uws/src/ProxyParser.h
    uws/src/QueryParser.h
    uws/src/TopicTree.h
    uws/src/Utilities.h
    uws/src/WebSocket.h
    uws/src/WebSocketContext.h
    uws/src/WebSocketContextData.h
    uws/src/WebSocketData.h
    uws/src/WebSocketExtensions.h
    uws/src/WebSocketHandshake.h
    uws/src/WebSocketProtocol.h
)

add_dependencies(uws boringssl)
target_link_libraries(uws PUBLIC ${BORINGSSL_STATIC_LIBRARIES})
target_include_directories(uws PUBLIC uws/src uws/uSockets/src uws/uSockets/boringssl/include)
target_compile_definitions(uws PUBLIC LIBUS_USE_OPENSSL)
