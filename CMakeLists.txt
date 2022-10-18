project(libtarparse CXX)
cmake_minimum_required(VERSION 3.18)

#  Library target
add_library(tarparse INTERFACE)
target_include_directories(tarparse INTERFACE ./src/include/)

#  Test suite
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
    tarparse)
