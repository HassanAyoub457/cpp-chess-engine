cmake_minimum_required(VERSION 3.16)

project(personal_chess_engine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(chess_engine_lib
    src/Board.cpp
    src/Engine.cpp
)

target_include_directories(chess_engine_lib PUBLIC include)

if(MSVC)
    target_compile_options(chess_engine_lib PRIVATE /W4 /permissive-)
else()
    target_compile_options(chess_engine_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(chess_engine src/main.cpp)
target_link_libraries(chess_engine PRIVATE chess_engine_lib)

include(CTest)
if(BUILD_TESTING)
    add_executable(chess_engine_tests tests/test_engine.cpp)
    target_link_libraries(chess_engine_tests PRIVATE chess_engine_lib)
    add_test(NAME chess_engine_tests COMMAND chess_engine_tests)
endif()
