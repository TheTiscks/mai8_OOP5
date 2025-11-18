cmake_minimum_required(VERSION 3.10)
project(lab_05)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_subdirectory(libs/googletest)

add_library(stack
    src/stack.cpp
)

add_executable(main_app main.cpp)
target_link_libraries(main_app stack)

add_executable(tests tests/test_stack.cpp src/stack.cpp)
target_link_libraries(tests stack gtest gtest_main)

enable_testing()
add_test(NAME StackTests COMMAND tests)