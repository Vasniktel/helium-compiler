cmake_minimum_required(VERSION 3.14)
project(helium)

set(CMAKE_CXX_STANDARD 11)

add_executable(helium main.cpp)

add_compile_options(-Wall -Wextra -Werror)

add_subdirectory(compiler)
add_subdirectory(compiler/tests)
target_link_libraries(helium compiler)
