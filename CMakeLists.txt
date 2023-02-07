
cmake_minimum_required(VERSION 3.21)
project(parka)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options (-fdiagnostics-color=always)

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options (-fcolor-diagnostics)

endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZLIB REQUIRED zlib)

include(uws.cmake)
include(loguru.cmake)

add_executable(parka main.cpp
    parka/optparse.h
    parka/AsyncFileReader.h
    parka/AsyncFileStreamer.h
)

target_include_directories(parka PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(parka PUBLIC ${ZLIB_INCLUDE_DIRS})
target_link_libraries(parka PUBLIC uws fmt loguru ${ZLIB_LDFLAGS})
target_compile_definitions(parka PUBLIC LIBUS_USE_OPENSSL)
