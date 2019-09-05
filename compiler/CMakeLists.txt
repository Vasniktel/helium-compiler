cmake_minimum_required(VERSION 3.14)
project(helium)

set(CMAKE_CXX_STANDARD 11)

add_library(compiler STATIC "")

add_subdirectory(abseil-cpp)
target_include_directories(compiler PRIVATE src)
target_include_directories(compiler PUBLIC include)

target_compile_definitions(compiler PRIVATE HELIUM_COMPILER_DEBUG)

target_sources(compiler
        PRIVATE
        src/compiler.cpp
        src/parser/parser.cpp
        src/parser/parser.hpp
        src/debug.cpp
        src/debug.hpp
        src/parser/ast.hpp
        src/parser/token.hpp
        src/parser/token.cpp
        src/module.cpp
        src/module.hpp
        src/error_reporter.cpp
        src/error_reporter.hpp
        src/parser/ast_printer.cpp
        src/parser/ast_printer.hpp

        PUBLIC
        include/compiler.hpp)

target_compile_options(compiler PRIVATE -Wall -Wextra -Werror -fno-exceptions -fno-rtti)
target_link_libraries(compiler
        absl::strings
        absl::flat_hash_map
        absl::str_format)
