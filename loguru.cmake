
include(fmt.cmake)

add_library(loguru STATIC
    loguru/loguru.hpp
    loguru/loguru.cpp
)

target_compile_definitions(loguru PUBLIC LOGURU_USE_FMTLIB=1)
target_link_libraries(loguru PUBLIC fmt)

