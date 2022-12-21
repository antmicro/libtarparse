cmake_minimum_required(VERSION 3.18)
project(libtarparse
    LANGUAGES CXX
    DESCRIPTION "A C++ tar parsing libary"
    VERSION 0.0.1
)

#  Library target
add_library(tarparse INTERFACE)
add_library(libtarparse::tarparse ALIAS tarparse)
target_include_directories(tarparse INTERFACE ./src/include/)

#  Test suite
if(DEFINED TARPARSE_COMPILE_TESTS)
    add_executable(tarparse_tests
        src/test/main.cpp
        src/test/test-tar-parser.cpp
        src/test/test-num-parser.cpp
    )
    target_compile_options(tarparse_tests PRIVATE
        --std=c++17 -Wall -Wextra -Werror=return-type
        -g -ggdb
        -fsanitize=address
    )
    target_link_options(tarparse_tests PRIVATE
        -fsanitize=address
    )
    target_link_libraries(tarparse_tests PRIVATE
        tarparse
    )
endif()

# Fuzzing target
# Assumes that clang is used as a compiler
if(DEFINED TARPARSE_COMPILE_FUZZ)
    add_executable(tarparse_fuzzer
        src/fuzz/target.cpp
    )
    target_compile_options(tarparse_fuzzer PRIVATE
        --std=c++17 -Wall -Wextra -Werror=return-type
        -g -ggdb
        -fsanitize=address,fuzzer
    )
    target_link_options(tarparse_fuzzer PRIVATE
        -fsanitize=address,fuzzer
    )
    target_link_libraries(tarparse_fuzzer PRIVATE
        tarparse
    )
endif()
